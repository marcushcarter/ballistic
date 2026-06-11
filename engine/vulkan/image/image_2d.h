#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace Ballistic::Vulkan
{
struct Image2DDesc
{
    VkExtent2D extent = {};
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t mipLevels = 1;
    uint32_t layers = 1;
    const char* debugName = nullptr;
};

struct Image2D
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {};
    VkImageUsageFlags usage = 0;
    uint32_t mipLevels = 1;
    uint32_t layers = 1;

    uint32_t bindlessSampled = UINT32_MAX;
    uint32_t bindlessStorage = UINT32_MAX;
    
    VkPipelineStageFlags stage = 0;
    VkAccessFlags access = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    
    bool Create(VkDevice device, VmaAllocator vma, const Image2DDesc& desc);
    bool WrapSwapchainImage(VkDevice device, VkImage swapchainImage, VkFormat swapchainFormat, VkExtent2D swapchainExtent);
    bool Resize(VkExtent2D newExtent);
    void Destroy();

    bool CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize bufferOffset = 0, VkOffset3D imageOffset = {0,0,0});
    bool CopyImage(VkCommandBuffer cmd, VkImage srcImage, VkExtent2D copyExtent, VkImageAspectFlags srcAspect = VK_IMAGE_ASPECT_DEPTH_BIT);

    void Transition(VkCommandBuffer cmd, VkImageLayout layout, VkPipelineStageFlags stage, VkAccessFlags access, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevels = 1);
    
    VkImage GetImage() const { return image; }
    VkImageView GetView() const { return view; }
    VmaAllocation GetAllocation() const { return allocation; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VmaAllocator vmaHandle = VK_NULL_HANDLE;
    bool ownsImage = true;
};
}