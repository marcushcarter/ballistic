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
        (void)ui;
        (void)cmd;
        (void)g;
    };

    return Ok; 
}

void EditorRenderPath::destroy_resources()
{
    RenderPath::destroy_resources();
}

void EditorRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}