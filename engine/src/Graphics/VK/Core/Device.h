#pragma once
#include "pch.h"

struct Device
{
    VkDevice device = VK_NULL_HANDLE;

    bool Create(VkPhysicalDevice physicalDevice, uint32_t graphicsFamily, uint32_t presentFamily = 0, uint32_t transferFamily = 0, const std::vector<const char*>& requiredExtensions = {});
    void Destroy();
    
    void Wait();

    VkDevice Get() const { return device; }
};
