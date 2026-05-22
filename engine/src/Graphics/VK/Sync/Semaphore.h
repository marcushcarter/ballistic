#pragma once
#include "pch.h"

struct Semaphore
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    
    bool Create(VkDevice device);
    void Destroy();
    
    VkSemaphore Get() const { return semaphore; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};