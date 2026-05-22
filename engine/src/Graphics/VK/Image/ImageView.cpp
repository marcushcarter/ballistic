#include "ImageView.h"

bool ImageView::Create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageViewType type, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);
    // VK_CHECK_HANDLE(image, VkImage, false);
    
    Destroy();
    deviceHandle = device;

    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = type;
    createInfo.format = format;
    createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    createInfo.subresourceRange.aspectMask = aspect;
    createInfo.subresourceRange.baseMipLevel = baseMip;
    createInfo.subresourceRange.levelCount = mipCount;
    createInfo.subresourceRange.baseArrayLayer = baseLayer;
    createInfo.subresourceRange.layerCount = layerCount;

    if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan image view");
        return false;
    }

    LOG_DEBUG("Image View created");
    return true;
}

void ImageView::Destroy()
{
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(deviceHandle, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Image View destroyed");
    }
}
