#include "transient_heap.h"

static VkDeviceSize AlignUp(VkDeviceSize v, VkDeviceSize a) { return (v + a - 1) & ~(a - 1); }

static uint64_t HashRequests(const std::vector<TransientRequest>& reqs)
{
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](const void* p, size_t n) {
        const uint8_t* b = (const uint8_t*)p;
        for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    };
    for (const auto& r : reqs) {
        mix(&r.kind, sizeof(r.kind));
        mix(&r.firstPass, sizeof(r.firstPass));
        mix(&r.lastPass,  sizeof(r.lastPass));
        if (r.kind == TransientRequest::Kind::Image) {
            mix(&r.imageDesc.format, sizeof(VkFormat));
            mix(&r.imageDesc.usage, sizeof(VkImageUsageFlags));
            mix(&r.imageDesc.aspect, sizeof(VkImageAspectFlags));
            mix(&r.imageDesc.samples, sizeof(VkSampleCountFlagBits));
            mix(&r.imageDesc.mipLevels, sizeof(uint32_t));
            mix(&r.imageDesc.layers, sizeof(uint32_t));
            mix(&r.extent, sizeof(VkExtent2D));
        } else {
            mix(&r.bufferDesc.size, sizeof(VkDeviceSize));
            mix(&r.bufferDesc.usage, sizeof(VkBufferUsageFlags));
        }
    }
    return h;
}

void TransientHeap::Init(VkDevice d, VmaAllocator a) { device = d; vma = a; }

void TransientHeap::Shutdown()
{
    for (auto& img : images)  img.Destroy(device);
    for (auto& buf : buffers) buf.Destroy(device);
    for (auto& b : buckets)   if (b.backing) vmaFreeMemory(vma, b.backing);
    images.clear(); buffers.clear(); buckets.clear();

    Recycle(UINT64_MAX);
    slots.clear();
    currentHash = 0; realizedOnce = false;
}

bool TransientHeap::Realize(const std::vector<TransientRequest>& requests, uint64_t frameIndex)
{
    const uint64_t hash = HashRequests(requests);
    if (realizedOnce && hash == currentHash) return true;
    currentHash = hash;
    realizedOnce = true;

    RetireCurrent(frameIndex);

    const size_t n = requests.size();
    slots.assign(n, {});
    images.clear();
    images.reserve(n);
    buffers.clear();
    buffers.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        const TransientRequest& r = requests[i];
        Slot& s = slots[i];
        s.kind = r.kind; s.firstPass = r.firstPass; s.lastPass = r.lastPass;

        if (r.kind == TransientRequest::Kind::Image) {
            PhysicalImage img{};
            if (!img.CreateUnbound(device, r.imageDesc, r.extent)) return false;
            s.physIndex = (uint32_t)images.size();
            s.memReq    = img.memReq;
            images.push_back(img);
        } else {
            PhysicalBuffer buf{};
            if (!buf.CreateUnbound(device, r.bufferDesc)) return false;
            s.physIndex = (uint32_t)buffers.size();
            s.memReq    = buf.memReq;
            buffers.push_back(buf);
        }
    }

    buckets.clear();
    for (size_t i = 0; i < n; ++i) {
        Slot& s = slots[i];
        uint32_t b = FindOrCreateBucket(s.kind, s.memReq.memoryTypeBits);
        buckets[b].members.push_back((uint32_t)i);
        s.bucket = b;
    }

    stats = {};
    for (Bucket& b : buckets) {
        b.size = PlaceBucket(b);
        if (!EnsureBacking(b)) return false;
        stats.totalBackingBytes += b.size;
    }

    for (size_t i = 0; i < n; ++i) {
        Slot& s = slots[i];
        VmaAllocation backing = buckets[s.bucket].backing;
        if (s.kind == TransientRequest::Kind::Image) {
            if (!images[s.physIndex].BindAndView(device, vma, backing, s.offset)) return false;
            stats.imageCount++;
        } else {
            if (!buffers[s.physIndex].Bind(vma, backing, s.offset)) return false;
            stats.bufferCount++;
        }
        stats.sumResourceBytes += s.memReq.size;
    }
    stats.bucketCount = (uint32_t)buckets.size();
    return true;
}

uint32_t TransientHeap::FindOrCreateBucket(TransientRequest::Kind kind, uint32_t typeBits)
{
    for (uint32_t i = 0; i < buckets.size(); ++i)
        if (buckets[i].kind == kind && buckets[i].memoryTypeBits == typeBits) return i;
    Bucket b{}; b.kind = kind; b.memoryTypeBits = typeBits;
    buckets.push_back(std::move(b));
    return (uint32_t)buckets.size() - 1;
}

VkDeviceSize TransientHeap::PlaceBucket(Bucket& b)
{
    std::sort(b.members.begin(), b.members.end(), [&](uint32_t a, uint32_t c) { return slots[a].firstPass < slots[c].firstPass; });

    struct FreeRange { VkDeviceSize offset, size; uint32_t freeAtPass; };
    std::vector<FreeRange> freeList;
    VkDeviceSize high = 0;
    b.maxAlign = 1;

    for (uint32_t mi : b.members) {
        Slot& s = slots[mi];
        const VkDeviceSize sz = s.memReq.size, al = s.memReq.alignment;
        b.maxAlign = std::max(b.maxAlign, al);

        int best = -1; VkDeviceSize bestOff = 0;
        for (int f = 0; f < (int)freeList.size(); ++f) {
            if (freeList[f].freeAtPass >= s.firstPass) continue;   // occupant still live -> cannot reuse
            const VkDeviceSize off = AlignUp(freeList[f].offset, al);
            if (off + sz <= freeList[f].offset + freeList[f].size &&
                (best < 0 || freeList[f].size < freeList[best].size)) { best = f; bestOff = off; }
        }
        if (best >= 0) { s.offset = bestOff; freeList.erase(freeList.begin() + best); }
        else           { s.offset = AlignUp(high, al); high = s.offset + sz; }

        freeList.push_back({ s.offset, sz, s.lastPass });          // reusable once this resource dies
    }
    return high;
}

bool TransientHeap::EnsureBacking(Bucket& b)
{
    VkMemoryRequirements req{};
    req.size = b.size;
    req.alignment = b.maxAlign;
    req.memoryTypeBits = b.memoryTypeBits;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_UNKNOWN;
    aci.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;    // transient RTs/scratch are device-local

    if (vmaAllocateMemory(vma, &req, &aci, &b.backing, nullptr) != VK_SUCCESS) {
        LOG_ERROR("TransientHeap: backing alloc failed (%llu bytes, typeBits 0x%x)", (unsigned long long)b.size, b.memoryTypeBits);
        return false;
    }
    return true;
}

void TransientHeap::RetireCurrent(uint64_t frameIndex)
{
    for (auto& img : images) retiredImages.push_back({ frameIndex, img.image, img.view });
    for (auto& buf : buffers) retiredBuffers.push_back({ frameIndex, buf.buffer });
    for (auto& b : buckets) if (b.backing) retiredBackings.push_back({ frameIndex, b.backing });
    images.clear();
    buffers.clear();
    buckets.clear();
}

void TransientHeap::Recycle(uint64_t completed)
{
    auto sweep = [completed](auto& vec, auto&& destroy) {
        size_t w = 0;
        for (size_t r = 0; r < vec.size(); ++r) {
            if (vec[r].frame <= completed) destroy(vec[r]);
            else vec[w++] = vec[r];
        }
        vec.resize(w);
    };
    
    sweep(retiredImages,  [&](RetiredImage& e)  { if (e.view) vkDestroyImageView(device, e.view, nullptr); if (e.image) vkDestroyImage(device, e.image, nullptr); });
    sweep(retiredBuffers, [&](RetiredBuffer& e) { if (e.buffer) vkDestroyBuffer(device, e.buffer, nullptr); });
    sweep(retiredBackings,[&](RetiredBacking& e){ if (e.backing) vmaFreeMemory(vma, e.backing); });
}

void TransientHeap::DumpStats() const
{
    const VkDeviceSize saved = stats.sumResourceBytes > stats.totalBackingBytes ? stats.sumResourceBytes - stats.totalBackingBytes : 0;
    LOG_DEBUG("TransientHeap: %u img, %u buf, %u buckets | backing %.2f MB | naive %.2f MB | saved %.2f MB (%.0f%%)",
        stats.imageCount, stats.bufferCount, stats.bucketCount,
        stats.totalBackingBytes / (1024.0 * 1024.0),
        stats.sumResourceBytes  / (1024.0 * 1024.0),
        saved / (1024.0 * 1024.0),
        stats.sumResourceBytes ? (100.0 * saved / stats.sumResourceBytes) : 0.0
    );
}

PhysicalImage& TransientHeap::GetImage(uint32_t slot)
{
    // ASSERT(slots[slot].kind == TransientRequest::Kind::Image);
    return images[slots[slot].physIndex];
}

PhysicalBuffer& TransientHeap::GetBuffer(uint32_t slot)
{
    // ASSERT(slots[slot].kind == TransientRequest::Kind::Buffer);
    return buffers[slots[slot].physIndex];
}
