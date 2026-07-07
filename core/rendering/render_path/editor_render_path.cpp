#include <core/rendering/render_path/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;

    Error err = RenderPath::create_resources();
    if (err != Ok) return err;
    
    present_pass.name = "present";
    present_pass.setup = [](RenderGraph::Builder& b) {
        b.write_image("backbuffer", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };

    drivers::ImGuiDriver* ui = imgui;
    present_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");
        // log_write("PRESENT exec: bb=%p ui=%p bb->view=%p", (void*)bb, (void*)ui,
        //     (void*)(bb ? bb->image_view : nullptr));

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = bb->image_view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, { bb->extent.width, bb->extent.height-20 } };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 1;
        ri.pColorAttachments = &color;


        // log_write("PRESENT ri: renderArea=%ux%u view=%p layout=%u cmd=%p loadOp=%u",
        //     ri.renderArea.extent.width, ri.renderArea.extent.height,
        //     (void*)color.imageView, (unsigned)color.imageLayout,
        //     (void*)cmd, (unsigned)color.loadOp); fflush(stdout);

        // log_write("PRESENT: before beginrendering"); fflush(stdout);
        vkCmdBeginRendering(cmd, &ri);
        // log_write("PRESENT: before record_commands"); fflush(stdout);
        ui->record_commands(cmd);
        // log_write("PRESENT: after record_commands"); fflush(stdout);
        vkCmdEndRendering(cmd);
    };

    return Ok; 
}

void EditorRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}