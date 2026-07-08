#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp_pass.name = "temp";
    temp_pass.setup = [](RenderGraph::Builder& b) {
        b.write_image("final_image", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        // b.write_image("imp_depth", VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
        // b.write_image("imp_color", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    };

    drivers::ImGuiDriver* ui = imgui;
    temp_pass.execute = [ui](VkCommandBuffer cmd, RenderGraph& g) {
        (void)ui;
        (void)cmd;
        (void)g;
    };

    return Ok;
}

void RenderPath::destroy_resources() {}

void RenderPath::build(RenderGraph& g)
{
    g.add(&temp_pass);
    build_present(g);
}

}