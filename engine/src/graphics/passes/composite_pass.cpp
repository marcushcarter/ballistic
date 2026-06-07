#include <graphics/passes/composite_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool CompositePass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void CompositePass::DestroyResources()
{

}

void CompositePass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle lightDiffuseSss;
        ResourceHandle lightSpecular;
        ResourceHandle light;
    };

    PassData out = g.AddPass<PassData>("CompositePass",
    [&](RenderGraph& builder, PassData& data)
    {
        data.lightDiffuseSss = builder.ReadImage(fg.lightDiffuseSSSImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        builder.ReadImage(fg.lightDiffuseSSSImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.light = builder.CreateImage("LightImage", {
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "light",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        
        fg.lightImage = data.light;
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
