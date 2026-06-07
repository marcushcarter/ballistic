#include <graphics/passes/hbao_blur_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool HBAOBlurPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void HBAOBlurPass::DestroyResources()
{

}

void HBAOBlurPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle hbao;
        ResourceHandle depth;
        ResourceHandle hbaoBlurred;
    };
    
    PassData out = g.AddPass<PassData>("HBAOBlurPass",
    [&](RenderGraph& builder, PassData& data)
    {
        data.hbao = builder.ReadImage(fg.aoRaw,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.depth = builder.ReadImage(fg.mainZBuffer,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.hbaoBlurred = builder.CreateImage("AOBlurImage", {
            .format = VK_FORMAT_R8_UNORM,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .widthScale = 0.5f,
            .heightScale = 0.5f,
            .debugName = "AOBlurred",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.aoBlurred = data.hbaoBlurred;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g) {
        (void)cmd;
        (void)data;
        (void)g;

        // VkImageView view = g.GetImageView(data.dst);
        // VkExtent2D ext = g.GetImageExtent(data.dst);
        // if (!view) return;

        // VkRenderingAttachmentInfo depth{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        // depth.imageView = view;
        // depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        // depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // depth.clearValue.depthStencil = { 1.0f, 0 };

        // VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        // info.renderArea = { {0,0}, ext };
        // info.layerCount = 1;
        // info.colorAttachmentCount = 0;
        // info.pColorAttachments = nullptr;
        // info.pDepthAttachment = &depth;

        // vkCmdBeginRendering(cmd, &info);
        // vkCmdEndRendering(cmd);
    });

}
