#include "blit_to_swapchain_pass.h"
#include "graphics/vk/image/image_2d.h"
#include "graphics/renderer.h"

void BlitToSwapchainPass::Setup(RenderGraph& g)
{
    finalIn = g.ReadImage("finalImage", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    swapOut = g.WriteImage("swapchain", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
}

void BlitToSwapchainPass::Execute(VkCommandBuffer cmd, RenderGraph& g)
{
    VkImageView swapView = g.GetImageView(swapOut);
    VkExtent2D ext = g.GetImageExtent(swapOut);
    if (!swapView) return;

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

    VKViewportScissor(cmd, 0, 0, (float)ext.width, (float)ext.height);
    renderer->blitPipeline.Bind(cmd);
    renderer->blitPipeline.DescriptorSets(cmd, { renderer->finalImageInputSet.Get() });
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);
}
