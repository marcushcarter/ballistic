#pragma once
#include "pch.h"

struct Allocator
{
    VmaAllocator allocator = VK_NULL_HANDLE;
    
    bool Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    void Destroy();

uint32_t FindMemoryTypeIndex(VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags = 0) const;

    VmaAllocator Get() const { return allocator; }
};
