#pragma once
#include "pch.h"
#include "Buffer.h"

struct VertexBuffer : public Buffer
{
    void Bind(VkCommandBuffer cmd) {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
    }

    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkDeviceSize initialSize, bool isHostVisible = false, VkBufferUsageFlags extraUsage = 0) {
        return Buffer::Create(device, props, initialSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | extraUsage, isHostVisible);
    }
};
