#include <core/rendering/render_path/render_path.h>
#include <core/log/log.h>

namespace ballistic {

Error RenderPath::create_resources()
{
    using enum Error;
    
    temp_pass.name = "temp";
    temp_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("final_image", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });
        b.color_attachment("imp_color", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });
        b.depth_attachment("imp_depth", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());
    };

    temp_pass.execute = [](VkCommandBuffer cmd, RenderGraph& g) {
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