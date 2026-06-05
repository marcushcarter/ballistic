#pragma once
#include <vulkan/vulkan.h>

struct Semaphore
{
    VkSemaphore semaphore = VK_NULL_HANDLE;
    const char* debugName = nullptr;
    
    bool Create(VkDevice device, const char* debugName = nullptr);
    void Destroy();
    
    VkSemaphore Get() const { return semaphore; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};