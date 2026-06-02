#pragma once
#include "pch.h"

struct BindlessHeapDesc
{
    uint32_t sampledImages = 16384;
    uint32_t storageImages = 4096;
    uint32_t samplers = 256;
    const char* debugName = nullptr;
};

struct BindlessHeap
{
    enum : uint32_t {
        BINDING_SAMPLED = 0,
        BINDING_STORAGE = 1,
        BINDING_SAMPLER = 2,
    };

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;

    struct IndexAllocator {
        std::vector<uint32_t> freeList;
        uint32_t next = 0, cap = 0;
        uint32_t Acquire() {
            if (!freeList.empty()) { uint32_t i = freeList.back(); freeList.pop_back(); return i; }
            BE_ASSERT(next < cap);
            return next++;
        }
        void Free(uint32_t i) { freeList.push_back(i); }
    };
    IndexAllocator sampledAlloc, storageAlloc, samplerAlloc;

    bool Create(VkDevice device, const BindlessHeapDesc& desc);
    void Destroy();

    uint32_t RegisterSampledImage(VkImageView view);
    uint32_t RegisterStorageImage(VkImageView view);
    uint32_t RegisterSampler(VkSampler sampler);

    void FreeSampledImage(uint32_t i) { if (i != UINT32_MAX) sampledAlloc.Free(i); }
    void FreeStorageImage(uint32_t i) { if (i != UINT32_MAX) storageAlloc.Free(i); }

    VkDescriptorSetLayout GetLayout() const { return layout; }
    VkDescriptorSet GetSet() const { return set; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
