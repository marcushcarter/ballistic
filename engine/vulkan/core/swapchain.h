#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Ballistic::Vulkan
{
struct Swapchain
{
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};

    bool Create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D windowExtent, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    bool Resize(VkExtent2D newExtent, VkPresentModeKHR presentMode);
    void Destroy();

    VkSwapchainKHR Get() { return swapchain; }
    std::vector<VkImage> GetImages() const;

private:
    VkPhysicalDevice physicalDeviceHandle = VK_NULL_HANDLE;
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VkSurfaceKHR surfaceHandle = VK_NULL_HANDLE;
};
}