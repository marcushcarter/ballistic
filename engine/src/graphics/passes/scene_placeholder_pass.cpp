#include "scene_placeholder_pass.h"

ResourceHandle AddScenePlaceholderPass(RenderGraph& g)
{    
    struct PassData { ResourceHandle dst; };
    PassData out = g.AddPass<PassData>("ScenePlaceholderPass",
    [&](RenderGraph& builder, PassData& data) {
        data.dst = builder.WriteImage("finalImage", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        
        builder.CreateImage("Test Transient Image", {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        // builder.CreateBuffer("MyBuffer", {
        //     .size  = sizeof(float) * 1024,
        //     .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        // },
        // VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        // VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
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
