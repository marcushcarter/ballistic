#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp_pass.name = "temp";
    temp_pass.setup = [](RenderGraph::Builder& b) {
        b.write_image("final_image", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        
        b.write_image("imp_depth", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
        b.write_image("imp_color", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    };

    drivers::ImGuiDriver* ui = imgui;
    temp_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("final_image");        
        auto* imp_depth = g.image("imp_depth");
        auto* imp_color = g.image("imp_color");
        // log_write("TEMP EXTENTS final=%ux%u imp_color=%ux%u imp_depth=%ux%u renderArea=%ux%u",
        //     bb->extent.width, bb->extent.height,
        //     imp_color->extent.width, imp_color->extent.height,
        //     imp_depth->extent.width, imp_depth->extent.height,
        //     bb->extent.width, bb->extent.height);

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = bb->image_view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { { 0.0f, 0.0f, 1.0f, 1.0f } };

        VkRenderingAttachmentInfo color2{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color2.imageView = imp_color->image_view;
        color2.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color2.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color2.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color2.clearValue.color = { { 0.0f, 0.0f, 1.0f, 1.0f } };

        log_write("DEPTH DIAG: format=%d (D32=126 D24S8=129 D32S8=130) aspect=%d ext=%ux%u usage_has_ds_bit=?",
            (int)imp_depth->format, (int)imp_depth->aspect,
            imp_depth->extent.width, imp_depth->extent.height); fflush(stdout);
        VkRenderingAttachmentInfo depth_att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_att.imageView = imp_depth->image_view;
        depth_att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_att.clearValue.depthStencil = { 0.0f, 0 };

        VkRenderingAttachmentInfo color_attachments[2] = { color, color2 };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, { bb->extent.width, bb->extent.height } };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 2;
        ri.pColorAttachments = color_attachments;
        ri.pDepthAttachment = &depth_att;

        vkCmdBeginRendering(cmd, &ri);
        vkCmdEndRendering(cmd);
        
        (void)cmd;
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