#include <core/rendering/render_path/game_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/io/embedded_resource.h>
#include <core/log/log.h>

namespace ballistic {

Error GameRenderPath::create_resources()
{
    using enum Error;
    if (Error e = RenderPath::create_resources(); e != Ok) return e;

    present_pass.name = "present_pass";
    present_pass.category = "Present";
    present_pass.formats = { { dd->swapchain.format } };
    present_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.1f, 0.1f, 1.0f } });
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    };
    present_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto* bb = cl.graph->image("backbuffer");
        auto* final_image = cl.graph->image("final_image");

        cl.dd->command_render_set_viewport(cl.cmd, {{{0,0},bb->extent}});
        cl.dd->command_render_set_scissor(cl.cmd, {{{0,0},bb->extent}});
        
        cl.dd->command_bind_pipeline(cl.cmd, gamma_blit_pipeline);
        struct { uint32_t srcIndex, samplerIndex; } pc;
        pc.srcIndex = final_image->bindless_sampled;
        pc.samplerIndex = cl.dd->default_sampler.bindless_sampler;
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.draw("gamma_blit", 3);
    };
    
    editor_ui_pass.name = "editor_ui_pass";
    editor_ui_pass.category = "Present";
    editor_ui_pass.formats = { { dd->swapchain.format } };
    editor_ui_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_LOAD);
        b.read_all_images();
    };
    editor_ui_pass.execute = [this](RenderGraph::CommandList& cl) {
        imgui->record_commands(cl.cmd);
    };

    EmbeddedResource::Blob blit_vert_blob = EmbeddedResource::load(L"SHADERS_FULLSCREEN_VERT");
    EmbeddedResource::Blob blit_frag_blob = EmbeddedResource::load(L"SHADERS_GAMMA_BLIT_FRAG");
    VkShaderModule blit_vs = dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Vertex, .glsl = (const char*)blit_vert_blob.data, .glsl_size = blit_vert_blob.size, .name = "gamma_blit_vs" });
    VkShaderModule blit_fs = dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Fragment, .glsl = (const char*)blit_frag_blob.data, .glsl_size = blit_frag_blob.size, .name = "gamma_blit_fs" });

    VkRenderPass rp = graph->acquire_render_pass(present_pass);
    drivers::DeviceDriverVulkan::GraphicsPipelineCreateInfo pipeline_ci{};
    pipeline_ci.vertex_shader = blit_vs;
    pipeline_ci.fragment_shader = blit_fs;
    pipeline_ci.render_pass = rp;
    pipeline_ci.cull_mode = VK_CULL_MODE_NONE;
    pipeline_ci.name = "gamma_blit_pipeline";
    gamma_blit_pipeline = dd->graphics_pipeline_create(pipeline_ci);

    dd->shader_free(blit_vs);
    dd->shader_free(blit_fs);

    return Ok;
}

void GameRenderPath::destroy_resources()
{
    dd->pipeline_free(gamma_blit_pipeline);
    RenderPath::destroy_resources();
}

void GameRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); g.add(&editor_ui_pass); }

}