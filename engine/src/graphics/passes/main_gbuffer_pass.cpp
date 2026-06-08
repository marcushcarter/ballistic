#include <graphics/passes/main_gbuffer_pass.h>

#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool MainGBufferPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void MainGBufferPass::DestroyResources()
{

}

void MainGBufferPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData {
        ResourceHandle albedo;
        ResourceHandle normal;
        ResourceHandle material;
        ResourceHandle depth;
    };

    PassData out = g.AddPass<PassData>("MainGBufferPass",
    [&](RenderGraph& builder, PassData& data)
    {
        // data.depth = builder.ReadImage(fg.mainZBuffer,
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        // VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        // VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

        data.albedo = builder.CreateImage("GBufferAlbedo", {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "GBufferAlbedo",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        data.normal = builder.CreateImage("GBufferNormal", {
            .format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "GBufferNormal", 
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        data.material = builder.CreateImage("GBufferMaterial", {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .debugName = "GBufferMaterial",
        },
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        fg.gbuffer.albedo = data.albedo;
        fg.gbuffer.normal = data.normal;
        fg.gbuffer.material = data.material;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {
        VkExtent2D ext = g.GetImageExtent(data.albedo);

        VkRenderingAttachmentInfo albedoAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        albedoAttachment.imageView = g.GetImageView(data.albedo);
        albedoAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        albedoAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkRenderingAttachmentInfo normalAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        normalAttachment.imageView = g.GetImageView(data.normal);
        normalAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        normalAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkRenderingAttachmentInfo materialAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        materialAttachment.imageView = g.GetImageView(data.material);
        materialAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        materialAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        materialAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        materialAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        // VkRenderingAttachmentInfo depth{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        // depth.imageView = g.GetImageView(data.depth);
        // depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        // depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // depth.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingAttachmentInfo colorAttachments[3] = { albedoAttachment, normalAttachment, materialAttachment };
        
        VkRenderingInfo info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        info.renderArea = { {0,0}, ext };
        info.layerCount = 1;
        info.colorAttachmentCount = 3;
        info.pColorAttachments = colorAttachments;
        // info.pDepthAttachment = &depth;

        vkCmdBeginRendering(cmd, &info);
        vkCmdEndRendering(cmd);
    });

}
