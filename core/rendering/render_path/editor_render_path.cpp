#include <core/rendering/render_path/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;
    if (Error e = RenderPath::create_resources(); e != Ok) return e;

    editor_ui_pass.name = "editor_ui_pass";
    editor_ui_pass.category = "Swapchain";
    editor_ui_pass.formats = { { dd->swapchain.format } };
    editor_ui_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } });
        b.read_all_images();
    };
    editor_ui_pass.execute = [this](VkCommandBuffer cmd, RenderGraph& g) {
        (void)g; imgui->record_commands(cmd);
    };

    return Ok; 
}

void EditorRenderPath::build_present(RenderGraph& g) { g.add(&editor_ui_pass); }

}