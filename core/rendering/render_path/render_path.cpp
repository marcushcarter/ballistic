#include <core/rendering/render_path/render_path.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    // base: create shared features. e.g. gbuffer.create(*device_driver), hbao.create(...)
    // none yet — empty for the current slice.

    
    temp_pass.name = "temp";
    temp_pass.setup = [](RenderGraph::Builder& b) {
        b.write_image("final_image", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    };

    drivers::ImGuiDriver* ui = imgui;
    temp_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("final_image");

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = bb->image_view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { { 0.0f, 0.0f, 1.0f, 1.0f } };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, { bb->extent.width, bb->extent.height } };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 1;
        ri.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &ri);
        vkCmdEndRendering(cmd);
    };

    return Ok;
}

void RenderPath::destroy_resources()
{
    // base: destroy shared features in reverse.
}

void RenderPath::build(RenderGraph& g)
{
    // shared deferred chain (none yet):
    // gbuffer.add(g); hbao.add(g); lighting.add(g); ...

    g.add(&temp_pass);

    build_present(g);
}

}