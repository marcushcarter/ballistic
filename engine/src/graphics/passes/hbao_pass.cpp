#include <graphics/passes/hbao_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool HBAOPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void HBAOPass::DestroyResources()
{

}

void HBAOPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle depth;
        ResourceHandle normal;
        ResourceHandle hbao;
    };
    
    PassData out = g.AddPass<PassData>("HBAOPass",
    [&](RenderGraph& builder, PassData& data)
    {
        // data.depth = builder.ReadImage(fg.mainZBuffer,
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        // VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        // VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.normal = builder.ReadImage(fg.gbuffer.normal,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.hbao = builder.CreateImage("AORawImage", {
            .format = VK_FORMAT_R8_UNORM,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .widthScale = 0.5f,
            .heightScale = 0.5f,
            .debugName = "HBAOBlurredImage",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.aoRaw = data.hbao;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {
        VkExtent2D ext = g.GetImageExtent(data.hbao);

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = g.GetImageView(data.hbao);
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);
        vkCmdEndRendering(cmd);
    });

}
