#include <graphics/passes/light_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool LightPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void LightPass::DestroyResources()
{

}

void LightPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle depth;
        ResourceHandle albedo;
        ResourceHandle normal;
        ResourceHandle material;
        ResourceHandle hbaoBlurred;
        ResourceHandle lightDiffuse;
        ResourceHandle lightSpecular;
        // ResourceHandle shadowHeightfield;
        // ResourceHandle shadowCascade0;
        // ResourceHandle shadowCascade1;
        // ResourceHandle shadowPlayer;
        // ResourceHandle shadowSpotlight;
    };

    PassData out = g.AddPass<PassData>("LightPass",
    [&](RenderGraph& builder, PassData& data)
    {
        // data.depth = builder.ReadImage(fg.mainZBuffer,
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        // VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        // VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.albedo = builder.ReadImage(fg.gbuffer.albedo,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.normal = builder.ReadImage(fg.gbuffer.normal,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.material = builder.ReadImage(fg.gbuffer.material,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        data.hbaoBlurred = builder.ReadImage(fg.aoBlurred,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        
        // shadow heightfield
        // shadow cascades 0
        // shadow cascade 1
        // shadow player
        // shadow spotlight

        data.lightDiffuse = builder.CreateImage("LightDiffuseImage", {
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "light_diffuse",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        
        data.lightSpecular = builder.CreateImage("LightSpecularImage", {
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "light_specular",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.lightDiffuseImage = data.lightDiffuse;
        fg.lightSpecularImage = data.lightSpecular;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {
        VkExtent2D ext = g.GetImageExtent(data.lightDiffuse);

        VkRenderingAttachmentInfo diffuseAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        diffuseAttachment.imageView = g.GetImageView(data.lightDiffuse);
        diffuseAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        diffuseAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        diffuseAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        diffuseAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkRenderingAttachmentInfo specularAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        specularAttachment.imageView = g.GetImageView(data.lightSpecular);
        specularAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        specularAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        specularAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        specularAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkRenderingAttachmentInfo colorAttachments[2] = { diffuseAttachment, specularAttachment };
        
        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 2;
        info.pColorAttachments = colorAttachments;

        // vkCmdBeginRendering(cmd, &info);
        // vkCmdEndRendering(cmd);
        
        (void)cmd;
    });

}
