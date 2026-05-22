#pragma once
#include "pch.h"

struct Surface
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    bool Create(VkInstance instance, GLFWwindow* window);
    void Destroy();
    
    VkSurfaceKHR Get() const { return surface; }
    
private:
    VkInstance instanceHandle = VK_NULL_HANDLE;
};