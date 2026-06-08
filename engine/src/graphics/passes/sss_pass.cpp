#include <graphics/passes/sss_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool SSSPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void SSSPass::DestroyResources()
{

}

void SSSPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle lightDiffuse;
        ResourceHandle depth;
        ResourceHandle material;
        ResourceHandle lightDiffuseSss;
    };

    PassData out = g.AddPass<PassData>("SSSPass",
    [&](RenderGraph& builder, PassData& data)
    {
        // data.depth = builder.ReadImage(fg.mainZBuffer,
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        // VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        // VK_ACCESS_2_SHADER_READ_BIT);

        data.material = builder.ReadImage(fg.gbuffer.material,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_READ_BIT);

        data.lightDiffuse = builder.ReadImage(fg.lightDiffuseImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_READ_BIT);

        data.lightDiffuseSss = builder.CreateImage("LightDiffuseSSSImage", {
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "light_diffuse_sss"
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.lightDiffuseSssImage = data.lightDiffuseSss;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {    
        VkExtent2D ext = g.GetImageExtent(data.lightDiffuseSss);

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = g.GetImageView(data.lightDiffuseSss);
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &info);
        vkCmdEndRendering(cmd);
    });

}
