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
    present_pass.setup = [](RenderGraphBuilder& b) {
        b.write_image("backbuffer", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };

    drivers::ImGuiDriver* ui = imgui;
    present_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");

        VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color.imageView = bb->image_view;
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, { bb->extent.width, bb->extent.height } };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 1;
        ri.pColorAttachments = &color;

        vkCmdBeginRendering(cmd, &ri);
        ui->record_commands(cmd);
        vkCmdEndRendering(cmd);
    };

    return Ok;
}

void GameRenderPath::destroy_resources()
{
    RenderPath::destroy_resources();
}

void GameRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}