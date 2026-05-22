#pragma once
#include "pch.h"
#include "Buffer.h"

struct IndirectBuffer : public Buffer 
{
    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize, bool isHostVisible = false, VkBufferUsageFlags extraUsage = 0) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | extraUsage, isHostVisible);
    }
};
