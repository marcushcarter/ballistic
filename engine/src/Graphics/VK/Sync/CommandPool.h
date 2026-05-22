#pragma once
#include "pch.h"

struct CommandPool
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    void Destroy();
    
    void Reset(VkCommandPoolResetFlags flags);
    
    VkCommandPool Get() const { return commandPool; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};