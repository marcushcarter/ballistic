#include "blit_to_swapchain_pass.h"
#include "graphics/renderer.h"

ResourceHandle AddBlitToSwapchainPass(RenderGraph& g, Renderer* renderer)
{
    struct PassData { ResourceHandle src, dst; };
    PassData out = g.AddPass<PassData>("BlitToSwapchainPass",
    [&](RenderGraph& builder, PassData& data) {
        data.src = builder.ReadImage("finalImage", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
        data.dst = builder.WriteImage("swapchain", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    },
    [renderer](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {    
        VkImageView swapView = g.GetImageView(data.dst);
        VkExtent2D ext = g.GetImageExtent(data.dst);
        if (!swapView || !renderer) return;

        VkRenderingAttachmentInfo color{};
        color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color.imageView = swapView;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderingInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);

        ViewportScissor(cmd, 0, 0, (float)ext.width, (float)ext.height);
        renderer->blitPipeline.Bind(cmd);
        renderer->blitPipeline.DescriptorSets(cmd, { renderer->finalImageInputSet.Get() });
        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRendering(cmd);
    });
    return out.dst;
}
