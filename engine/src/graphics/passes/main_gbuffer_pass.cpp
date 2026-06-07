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
        data.depth = builder.ReadImage(fg.mainZBuffer,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

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
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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
