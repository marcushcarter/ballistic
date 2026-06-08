#include <graphics/passes/overlay_pass.h>
#include <graphics/render_graph/render_graph.h>
#include <graphics/render_graph/render_path.h>
#include <graphics/renderer.h>
#include <core/assert.h>
#include <resources.h>

bool OverlayPass::CreateResources(Renderer& r)
{
    renderer = &r;
    return true;
}

void OverlayPass::DestroyResources()
{

}

void OverlayPass::AddPass(RenderGraph& g, FrameGraph& fg)
{
    struct PassData { ResourceHandle finalImage; };
    PassData out = g.AddPass<PassData>("OverlayPass",
    [&](RenderGraph& builder, PassData& data)
    {
        data.finalImage = builder.ReadWriteImage(fg.finalImage,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        
        fg.finalImage = data.finalImage;
    },
    [this](VkCommandBuffer cmd, const PassData& data, RenderGraph& g)
    {
        VkExtent2D ext = g.GetImageExtent(data.finalImage);

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = g.GetImageView(data.finalImage);
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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
