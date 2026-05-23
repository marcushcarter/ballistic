#pragma once
#include "pch.h"

struct Pass
{
    // void Begin(VkCommandBuffer cmd, VkExtent2D renderExtent, std::vector<VkRenderingAttachmentInfo> renderingAttachments, VkRenderingAttachmentInfo depthAttachment = {});

    void Begin() {}
    void End() {}
};

inline VkRenderingAttachmentInfo PassAttachment(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, VkClearValue clearValue = {}) {
    VkRenderingAttachmentInfo a{};
    a.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    a.imageView = view;
    a.imageLayout = layout;
    a.loadOp = loadOp;
    a.storeOp = storeOp;
    a.clearValue = clearValue;
    return a;
}
