#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace Ballistic::Vulkan
{
struct AllocatorPool
{
    VmaPool pool = VK_NULL_HANDLE;

    bool Create(VmaAllocator allocator, uint32_t memoryTypeIndex, VkDeviceSize blockSize, uint32_t maxBlockCount, VmaPoolCreateFlags flags);
    void Destroy();

    VmaPool Get() const { return pool; }

private:
    VmaAllocator allocatorHandle = VK_NULL_HANDLE;
};
}