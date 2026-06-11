#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace Ballistic::Vulkan
{
struct Surface
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    bool Create(VkInstance instance, GLFWwindow* window);
    void Destroy();
    
    VkSurfaceKHR Get() const { return surface; }
    
private:
    VkInstance instanceHandle = VK_NULL_HANDLE;
};
}