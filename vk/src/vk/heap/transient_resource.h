#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

struct TransientImageDesc
{
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSampleCountFlagBits samples   = VK_SAMPLE_COUNT_1_BIT;
    uint32_t mipLevels = 1, layers = 1;

    enum class Sizing { ViewportRelative, Fixed };
    Sizing sizing = Sizing::ViewportRelative;
    float widthScale = 1.0f, heightScale = 1.0f;
    uint32_t fixedWidth = 0, fixedHeight = 0;

    const char* debugName = nullptr;
};

struct TransientBufferDesc
{
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const char* debugName = nullptr;
};

struct PhysicalImage
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkExtent2D extent = {};
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t mipLevels = 1, layers = 1;
    VkMemoryRequirements memReq = {};

    uint32_t bindlessSampled = UINT32_MAX;
    uint32_t bindlessStorage = UINT32_MAX;

    bool CreateUnbound(VkDevice device, const TransientImageDesc& desc, VkExtent2D resolved);
    bool BindAndView(VkDevice device, VmaAllocator vma, VmaAllocation backing, VkDeviceSize offset);
    void Destroy(VkDevice device);
};

struct PhysicalBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    VkMemoryRequirements memReq = {};

    bool CreateUnbound(VkDevice device, const TransientBufferDesc& desc);
    bool Bind(VmaAllocator vma, VmaAllocation backing, VkDeviceSize offset);
    void Destroy(VkDevice device);
};

