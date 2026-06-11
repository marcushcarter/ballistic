#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Ballistic::Vulkan
{
struct RenderPass
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkImageLayout> finalLayouts;

    bool Create(VkDevice device, const std::vector<VkAttachmentDescription>& attachments);
    void Destroy();

    void Begin(VkCommandBuffer cmd, VkFramebuffer framebuffer, VkExtent2D extent, const std::vector<VkClearValue>& clearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void End(VkCommandBuffer cmd);

    VkRenderPass Get() const { return renderPass; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};

inline VkAttachmentDescription RenderPassAttachment(VkFormat format, VkImageLayout layout, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, uint32_t samples = 1) {
    VkAttachmentDescription a{};
    a.format = format;
    a.samples = (VkSampleCountFlagBits)samples;
    a.loadOp = loadOp;
    a.storeOp = storeOp;
    a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    a.initialLayout = layout;
    a.finalLayout = layout;
    return a;
}
}