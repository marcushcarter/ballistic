#include <pass/framebuffer.h>

namespace Ballistic::Vulkan
{
bool Framebuffer::Create(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments, uint32_t layers)
{
    Destroy();
    deviceHandle = device;
    renderPassHandle = renderPass;

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.layers = layers;

    if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan framebuffer");
        return false;
    }

    // LOG_DEBUG("Framebuffer created");
    return true;
}

bool Framebuffer::Resize(VkExtent2D newExtent, const std::vector<VkImageView>& newAttachments, uint32_t newLayers)
{
    return Create(deviceHandle, renderPassHandle, newExtent, newAttachments, newLayers);
}

void Framebuffer::Destroy()
{
    if (framebuffer) {
        vkDestroyFramebuffer(deviceHandle, framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
        // LOG_DEBUG("Framebuffer destroyed");
    }
}
}