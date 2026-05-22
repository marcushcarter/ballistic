#pragma once
#include "pch.h"
#include "Buffer.h"

struct StagingBuffer : public Buffer
{
    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize, VkBufferUsageFlags extraUsage = 0) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | extraUsage, true);
    }
};
