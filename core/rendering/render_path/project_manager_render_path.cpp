#include <core/rendering/render_path/project_manager_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error ProjectManagerRenderPath::create_resources()
{
    using enum Error;
    ui_pass.name = "ProjectManagerUI";
    ui_pass.category = "Present";
    ui_pass.formats = { { dd->swapchain.format } };
    ui_pass.setup = [](RenderGraph::Builder& b) { b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } }); };
    ui_pass.execute = [this](RenderGraph::CommandList& cl) { imgui->record_commands(cl.cmd); };
    return Ok;
}


void ProjectManagerRenderPath::build(RenderGraph& g) { g.add(&ui_pass); }

}