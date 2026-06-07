#include <graphics/passes/main_z_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool MainZPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void MainZPass::DestroyResources()
{

}

void MainZPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData { ResourceHandle depth; };
    PassData out = g.AddPass<PassData>("MainZPass",
    [&](RenderGraph& builder, PassData& data)
    {
        data.depth = builder.CreateImage("MainZBuffer", {
            .format = VK_FORMAT_D32_SFLOAT,
            .usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
            .debugName = "MainZBuffer",
        },
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

        fg.mainZBuffer = data.depth;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        (void)cmd;

        VkImageView view = g.GetImageView(data.depth);
        VkExtent2D ext = g.GetImageExtent(data.depth);
        if (!view) return;

        VkRenderingAttachmentInfo depth{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth.imageView = view;
        depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 0;
        info.pColorAttachments = nullptr;
        info.pDepthAttachment = &depth;

        // vkCmdBeginRendering(cmd, &info);
        // vkCmdEndRendering(cmd);
    });

}
