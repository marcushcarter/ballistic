#pragma once
#include "pch.h"
#include "Buffer.h"

struct StorageBuffer : public Buffer
{
    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize, bool isHostVisible = false, VkBufferUsageFlags extraUsage = 0) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraUsage, isHostVisible);
    }
};
