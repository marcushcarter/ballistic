#pragma once
#include "pch.h"

struct DescriptorPool
{
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, uint32_t count = 1000);
    void Destroy();
    
    VkDescriptorPool Get() const { return descriptorPool; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
