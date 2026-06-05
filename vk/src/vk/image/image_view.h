#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

struct ImageView
{
    VkImageView imageView = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, uint32_t baseMip = 0, uint32_t mipCount = 1, uint32_t baseLayer = 0, uint32_t layerCount = 1);
    void Destroy();
    
    VkImageView Get() const { return imageView; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
