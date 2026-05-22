#pragma once
#include "pch.h"

struct Fence
{
    VkFence fence = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, bool signaled = true);
    void Destroy();
    
    void Wait(uint64_t timeout = UINT64_MAX);
    void Reset();
    
    VkFence Get() const { return fence; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};