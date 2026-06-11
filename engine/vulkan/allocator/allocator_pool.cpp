#include <allocator/allocator_pool.h>

namespace Ballistic::Vulkan
{
bool AllocatorPool::Create(VmaAllocator allocator, uint32_t memoryTypeIndex, VkDeviceSize blockSize, uint32_t maxBlockCount, VmaPoolCreateFlags flags)
{
    Destroy();
    allocatorHandle = allocator;

    VmaPoolCreateInfo createInfo{};
    createInfo.memoryTypeIndex = memoryTypeIndex;
    createInfo.blockSize = blockSize;
    createInfo.maxBlockCount = maxBlockCount;
    createInfo.flags = flags;

    if (vmaCreatePool(allocator, &createInfo, &pool) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create VMA pool");
        return false;
    }

    // LOG_DEBUG("Allocator Pool created");
    return true;
}

void AllocatorPool::Destroy()
{
    if (pool) {
        vmaDestroyPool(allocatorHandle, pool);
        pool = VK_NULL_HANDLE;
        // LOG_DEBUG("Allocator Pool destroyed");
    }
}
}