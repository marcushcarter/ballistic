#include <core/rendering/render_path/game_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/log/log.h>

namespace ballistic {

Error GameRenderPath::create_resources()
{
    using enum Error;

    Error err = RenderPath::create_resources();
    if (err != Ok) return err;
    
    present_pass.name = "present";
    present_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });
        b.read_all_images();
    };

    drivers::ImGuiDriver* ui = imgui;
    present_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");
        VkViewport vp{ 0, 0, (float)bb->extent.width, (float)bb->extent.height, 0.0f, 1.0f };
        VkRect2D sc{ {0,0}, { bb->extent.width, bb->extent.height } };
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &sc);

        ui->record_commands(cmd);
    };

    return Ok;
}

void GameRenderPath::destroy_resources()
{
    RenderPath::destroy_resources();
}

void GameRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}