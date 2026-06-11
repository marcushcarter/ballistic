#include <allocator/allocator.h>
#include <cstdint>

namespace Ballistic::Vulkan
{
bool Allocator::Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    Destroy();

    VmaAllocatorCreateInfo createInfo{};
    createInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    createInfo.physicalDevice = physicalDevice;
    createInfo.device = device;
    createInfo.instance = instance;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    
    if (vmaCreateAllocator(&createInfo, &allocator) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create VMA allocator pipeline");
        return false;
    }

    // LOG_DEBUG("Allocator created");
    return true;
}

void Allocator::Destroy()
{
    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
        // LOG_DEBUG("Allocator destroyed");
    }
}

uint32_t Allocator::FindMemoryTypeIndex(VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags) const
{
    VkBufferCreateInfo bufferCI{};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size  = 1;
    bufferCI.usage = bufferUsage;

    VmaAllocationCreateInfo allocCI{};
    allocCI.usage = memoryUsage;
    allocCI.flags = allocationFlags;

    uint32_t memoryTypeIndex = 0;
    vmaFindMemoryTypeIndexForBufferInfo(allocator, &bufferCI, &allocCI, &memoryTypeIndex);
    return memoryTypeIndex;
}
}