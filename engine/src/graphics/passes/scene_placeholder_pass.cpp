#include "scene_placeholder_pass.h"

ResourceHandle AddScenePlaceholderPass(RenderGraph& g)
{    
    struct PassData { ResourceHandle dst; };
    PassData out = g.AddPass<PassData>("ScenePlaceholderPass",
    [&](RenderGraph& builder, PassData& data) {
        data.dst = builder.WriteImage("finalImage", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    },
    [](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        VkImageView view = g.GetImageView(data.dst);
        VkExtent2D ext = g.GetImageExtent(data.dst);
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
    });
    return out.dst;
}
