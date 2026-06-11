#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace Ballistic::Vulkan
{
struct Framebuffer
{
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    bool Create(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments, uint32_t layers = 1);
    bool Resize(VkExtent2D newExtent, const std::vector<VkImageView>& newAttachments, uint32_t newLayers = 1);
    void Destroy();

    VkFramebuffer Get() const { return framebuffer; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VkRenderPass renderPassHandle = VK_NULL_HANDLE;
};
}