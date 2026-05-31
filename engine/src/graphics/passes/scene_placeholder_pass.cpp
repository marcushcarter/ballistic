#include "scene_placeholder_pass.h"
#include "graphics/vk/vk.h"

void ScenePlaceholderPass::Setup(RenderGraph& g)
{
    finalOut = g.WriteImage("finalImage", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
}

void ScenePlaceholderPass::Execute(VkCommandBuffer cmd, RenderGraph& g)
{
    VkImageView view = g.GetImageView(finalOut);
    VkExtent2D ext = g.GetImageExtent(finalOut);
    if (!view) return;

    VkRenderingAttachmentInfo color{};
    color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color.imageView = view;
    color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.clearValue.color = { sinf((float)glfwGetTime()), 0.0f, 1.0f, 1.0f };

    VkRenderingInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.renderArea = { {0,0}, ext };
    info.layerCount = 1;
    info.colorAttachmentCount = 1;
    info.pColorAttachments = &color;

    vkCmdBeginRendering(cmd, &info);
    vkCmdEndRendering(cmd);
}
