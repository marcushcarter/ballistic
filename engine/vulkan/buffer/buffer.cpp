#include <buffer/buffer.h>
#include <misc/utils.h>
#include <vector>

namespace Ballistic::Vulkan
{
inline VkDeviceSize NextPowerOfTwo(VkDeviceSize x)
{
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    if constexpr(sizeof(VkDeviceSize) == 8) x |= x >> 32;
    return x + 1;
}

bool Buffer::Create(VkDevice device, VmaAllocator vma, const BufferDesc& d)
{
    Destroy();
    usage = d.usage;
    hostVisible = d.hostVisible;
    debugName = d.debugName;
    size = 0;
    capacity = 0;
    deviceAddress = 0;
    deviceHandle = device;
    vmaHandle = vma;

    if (!hostVisible)
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    createInfo.size = std::max<VkDeviceSize>(1, d.size);
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    if (hostVisible) {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    } else {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    VmaAllocationInfo allocationInfo{};

    if (vmaCreateBuffer(vma, &createInfo, &allocInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
        // LOG_ERROR("Buffer create failed: %s - vmaCreateBuffer", debugName ? debugName : "Unnamed");
        return false;
    }

    capacity = createInfo.size;

    if (hostVisible)
        mappedPtr = allocationInfo.pMappedData;

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
        info.buffer = buffer;
        deviceAddress = vkGetBufferDeviceAddress(device, &info);
    }

    SetObjectName(device, VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer, d.debugName);

    // LOG_DEBUG("Buffer created: %s (%llu bytes, usage %s, %s)",
    //     debugName ? debugName : "Unnamed",
    //     static_cast<unsigned long long>(capacity),
    //     vk::to_string(vk::BufferUsageFlags(usage)).c_str(),
    //     hostVisible ? "Host Visible" : "Device Local"
    // );

    return true;
}

void Buffer::Destroy()
{
    mappedPtr = nullptr;
    deviceAddress = 0;
    if (buffer) {
        vmaDestroyBuffer(vmaHandle, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        // LOG_DEBUG("Buffer destroyed: %s", debugName ? debugName : "Unnamed");
    }
}

bool Buffer::Update(void* data, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (!hostVisible || !mappedPtr) {
        // LOG_WARN("Buffer update failed: %s - not host visible", debugName ? debugName : "Unnamed");
        return false;
    }

    VkDeviceSize end = offset + dataSize;
    bool resized = false;

    if (end > capacity) {
        resized = true;
        if (!Resize(end)) return false;
    }

    memcpy((uint8_t*)mappedPtr + offset, data, dataSize);
    size = std::max(size, end);
    return resized;
}

bool Buffer::Resize(VkDeviceSize newSize)
{
    if (!hostVisible || newSize <= 0) return false;

    VkDeviceSize newCapacity = NextPowerOfTwo(newSize);
    if (newCapacity <= capacity) return false;

    std::vector<uint8_t> existing(size);
    if (size > 0 && mappedPtr)
        memcpy(existing.data(), mappedPtr, static_cast<size_t>(size));

    VkDeviceSize oldSize = size;
        
    if (!Create(deviceHandle, vmaHandle, {
        .size = newCapacity,
        .usage = usage,
        .hostVisible = hostVisible,
        .debugName = debugName
    })) return false;

    if (oldSize > 0)
        memcpy(mappedPtr, existing.data(), static_cast<size_t>(oldSize));
    
    size = oldSize;
    return true;
}

bool Buffer::Copy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize copySize, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
    if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)) {
        // LOG_WARN("Buffer create failed: %s - missing TRANSFER_DST_BIT", debugName ? debugName : "Unnamed");
        return false;
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = copySize;

    vkCmdCopyBuffer(cmd, srcBuffer, buffer, 1, &copyRegion);
    return true;
}

void Buffer::Transition(VkCommandBuffer cmd, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess)
{
    VkPipelineStageFlags srcStage = stage ? stage : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags srcAccess = access;
    
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);

    stage = dstStage;
    access = dstAccess;
}

void Buffer::BindIndex(VkCommandBuffer cmd) { vkCmdBindIndexBuffer(cmd, buffer, 0, VK_INDEX_TYPE_UINT32); }

void Buffer::BindVertex(VkCommandBuffer cmd)
{
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
}
}