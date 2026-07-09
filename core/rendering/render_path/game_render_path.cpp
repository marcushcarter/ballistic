#include <core/rendering/render_path/game_render_path.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/io/embedded_resource.h>
#include <core/log/log.h>

namespace ballistic {

Error GameRenderPath::create_resources()
{
    using enum Error;

    Error err = RenderPath::create_resources();
    if (err != Ok) return err;

    EmbeddedResource::Blob blit_vert_blob = EmbeddedResource::load(L"SHADERS_FULLSCREEN_VERT");
    EmbeddedResource::Blob blit_frag_blob = EmbeddedResource::load(L"SHADERS_GAMMA_BLIT_FRAG");

    VkShaderModule blit_vs = device_driver->shader_create({
        .stage = drivers::DeviceDriverVulkan::ShaderStage::Vertex,
        .glsl_source = (const char*)blit_vert_blob.data,
        .name = "blit_vs",
    });
    
    VkShaderModule blit_fs = device_driver->shader_create({
        .stage = drivers::DeviceDriverVulkan::ShaderStage::Vertex,
        .glsl_source = (const char*)blit_frag_blob.data,
        .name = "blit_fs",
    });

    // PIPELINE

    device_driver->shader_free(blit_vs);
    device_driver->shader_free(blit_fs);
    
    struct PassData {
        drivers::DeviceDriverVulkan* device_driver;
        drivers::ImGuiDriver* imgui;
        VkPipeline blit_pipeline;
    } data;

    data.device_driver = device_driver;
    data.imgui = imgui;
    data.blit_pipeline = VK_NULL_HANDLE;

    present_pass.name = "present";
    present_pass.setup = [](RenderGraph::Builder& b) {
        b.color_attachment("backbuffer", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });
        b.read_image("final_image", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        // b.read_all_images();
    };
    present_pass.execute = [data](VkCommandBuffer cmd, RenderGraph& g) {
        auto* bb = g.image("backbuffer");
        VkViewport vp{ 0, 0, (float)bb->extent.width, (float)bb->extent.height, 0.0f, 1.0f };
        VkRect2D sc{ {0,0}, { bb->extent.width, bb->extent.height } };
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &sc);

        data.imgui->record_commands(cmd);
    };

    return Ok;
}

void GameRenderPath::destroy_resources()
{
    // destroy
    RenderPath::destroy_resources();
}

void GameRenderPath::build_present(RenderGraph& g) { g.add(&present_pass); }

}