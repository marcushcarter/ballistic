#include <core/rendering/render_path/editor_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error EditorRenderPath::create_resources()
{
    using enum Error;
    if (Error e = RenderPath::create_resources(); e != Ok) return e;

    present_pass.name = "present";
    present_pass.formats = {
        { device_driver->swapchain.format }
    };

    present_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } });
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        b.read_all_images();
    };

    present_pass.execute = [this](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");
        VkViewport vp{ 0, 0, (float)bb->extent.width, (float)bb->extent.height, 0.0f, 1.0f };
        VkRect2D sc{ {0,0}, { bb->extent.width, bb->extent.height } };
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &sc);

        imgui->record_commands(cmd);
    };

    return Ok; 
}

void EditorRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}