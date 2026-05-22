#pragma once
#include "pch.h"
#include "Buffer.h"

struct UniformBuffer : public Buffer
{
    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
    }
};
