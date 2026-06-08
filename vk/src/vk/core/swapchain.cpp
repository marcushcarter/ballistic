#include <vk/core/swapchain.h>
#include <algorithm>
// #include "graphics/vk/misc/utils.h"

bool Swapchain::Create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D windowExtent, bool vsyncEnabled, VkSwapchainKHR oldSwapchain)
{
    vsync = vsyncEnabled;
    physicalDeviceHandle = physicalDevice;
    deviceHandle = device;
    surfaceHandle = surface;
    extent = windowExtent;
    
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

    extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

    VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
    for (const auto& f : surfaceFormats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = f;
            break;
        }
    }
    format = surfaceFormat.format;

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!vsync) {
        bool picked = false;
        for (auto m : presentModes) if (m == VK_PRESENT_MODE_IMMEDIATE_KHR) { presentMode = m; picked = true; break; }
        if (!picked)
            for (auto m : presentModes) if (m == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = m; break; }
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan swapchain");
        return false;
    }

    return true;
}

bool Swapchain::Resize(VkExtent2D newExtent, bool newVsync)
{
    VkSwapchainKHR oldSwapchain = swapchain;

    extent = newExtent;
    if (!Create(physicalDeviceHandle, deviceHandle, surfaceHandle, extent, newVsync, oldSwapchain)) {
        // LOG_ERROR("Failed to resize Vulkan swapchain");
        return false;
    }

    if (oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(deviceHandle, oldSwapchain, nullptr);

    return true;
}

void Swapchain::Destroy()
{
    if (swapchain) {
        vkDestroySwapchainKHR(deviceHandle, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
        // LOG_DEBUG("Swapchain destroyed");
    }
}

std::vector<VkImage> Swapchain::GetImages() const
{
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(deviceHandle, swapchain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(deviceHandle, swapchain, &count, images.data());
    return images;
}
