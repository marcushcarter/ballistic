#pragma once
#include "pch.h"
#include "Buffer.h"

struct IndexBuffer : public Buffer
{
    VkIndexType type = VK_INDEX_TYPE_UINT32;
    
    void Bind(VkCommandBuffer cmd) {
        vkCmdBindIndexBuffer(cmd, buffer, 0, type);
    }

    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize, bool isHostVisible = false, VkBufferUsageFlags extraUsage = 0) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | extraUsage, isHostVisible);
    }
};
