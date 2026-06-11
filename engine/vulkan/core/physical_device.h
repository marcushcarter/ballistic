#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Ballistic::Vulkan
{
struct PhysicalDevice
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceMemoryProperties memory{};

    int graphicsFamily = -1;
    int presentFamily = -1;
    int transferFamily = -1;
    int computeFamily = -1;

    bool Pick(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);
    bool HasDedicatedCompute() const { return computeFamily != graphicsFamily; }

    VkPhysicalDevice Get() const { return physicalDevice; }
};
}