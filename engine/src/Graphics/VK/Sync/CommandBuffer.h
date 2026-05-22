#pragma once
#include "pch.h"

struct CommandBuffer
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    
    bool Allocate(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void Free();

    bool Begin(VkCommandBufferUsageFlags flags = 0);
    void End();
    void Reset();
    
    VkCommandBuffer Get() const { return commandBuffer; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VkCommandPool commandPoolHandle = VK_NULL_HANDLE;
};