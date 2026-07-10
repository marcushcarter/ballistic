#include <core/rendering/render_path/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;
    if (Error e = RenderPath::create_resources(); e != Ok) return e;

    present_pass.name = "present";
    present_pass.category = "Swapchain";
    present_pass.formats = {
        { dd->swapchain.format }
    };

    present_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } });
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_all_images();
    };

    present_pass.execute = [this](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");

        dd->command_render_set_viewport(cmd, {{{0,0},bb->extent}});
        dd->command_render_set_scissor(cmd, {{{0,0},bb->extent}});
        
        imgui->record_commands(cmd);
    };

    return Ok; 
}

void EditorRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}