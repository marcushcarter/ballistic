#include <core/rendering/render_path/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;
    if (Error e = SceneRenderPath::create_resources(); e != Ok) return e;

    ui_pass.name = "Editor_UI";
    ui_pass.category = "Present";
    ui_pass.formats = { { dd->swapchain.format } };
    ui_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("Backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } });
        b.read_image("Out_Color", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_image("HDR_Color", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };
    ui_pass.execute = [this](RenderGraph::CommandList& cl) {
        imgui->record_commands(cl.cmd);
    };

    screenshot.dd = dd;
    screenshot.graph = graph;
    screenshot.create_resources();

    return Ok; 
}

void EditorRenderPath::destroy_resources()
{
    screenshot.destroy_resources();
    SceneRenderPath::destroy_resources();
}


void EditorRenderPath::build_present(RenderGraph& g) {
    g.add(&ui_pass);
    screenshot.build(g);
}

}