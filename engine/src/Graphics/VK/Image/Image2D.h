#pragma once
#include "pch.h"

struct Image2D
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};
    VkImageUsageFlags usage = 0;
    uint32_t mipLevels = 1;
    uint32_t layers = 1;
    
    VkPipelineStageFlags stage = 0;
    VkAccessFlags access = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    
    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& props, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevels = 1, uint32_t layers = 1);
    bool WrapSwapchainImage(VkDevice device, VkImage image, VkFormat format, VkExtent2D extent, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT);
    bool Resize(VkExtent2D newExtent);
    void Destroy();

    bool CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize bufferOffset = 0, VkOffset3D imageOffset = {0,0,0});
    bool CopyImage(VkCommandBuffer cmd, VkImage srcImage, VkExtent2D copyExtent, VkImageAspectFlags srcAspect = VK_IMAGE_ASPECT_DEPTH_BIT);

    void Transition(VkCommandBuffer cmd, VkImageLayout layout, VkPipelineStageFlags stage, VkAccessFlags access, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevels = 1);
    
    VkImage GetImage() const { return image; }
    VkImageView GetView() const { return imageView; }
    VkDeviceMemory GetMemory() const { return memory; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    const VkPhysicalDeviceMemoryProperties* memoryProps = nullptr;
    bool ownsImage = true;
};
