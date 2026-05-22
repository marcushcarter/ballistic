#include "Swapchain.h"

bool Swapchain::Create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D ext, bool vsync, VkSwapchainKHR oldSwapchain)
{
    // VK_CHECK_HANDLE(physicalDevice, VkPhysicalDevice, false);
    // VK_CHECK_HANDLE(device, VkDevice, false);
    // VK_CHECK_HANDLE(surface, VkSurfaceKHR, false);

    physicalDeviceHandle = physicalDevice;
    deviceHandle = device;
    surfaceHandle = surface;
    extent = ext;
    
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

    extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSurfaceFormatKHR surfaceFormat{};
    surfaceFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    format = surfaceFormat.format;

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
        LOG_ERROR("Failed to create Vulkan swapchain");
        return false;
    }

    LOG_DEBUG("Swapchain created: %dx%d", extent.width, extent.height);
    return true;
}

bool Swapchain::Resize(VkExtent2D newExtent, bool vsync)
{
    VkSwapchainKHR oldSwapchain = swapchain;

    extent = newExtent;
    if (!Create(physicalDeviceHandle, deviceHandle, surfaceHandle, extent, vsync, oldSwapchain)) {
        LOG_DEBUG("Failed to resize Vulkan swapchain");
        return false;
    }

    if (oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(deviceHandle, oldSwapchain, nullptr);

    return true;
}

void Swapchain::Destroy()
{
    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(deviceHandle, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
        physicalDeviceHandle = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        surfaceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Swapchain destroyed");
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
