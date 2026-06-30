#include <core/application/application.h>
#include <core/rendering/render_path.h>
#include <core/log/error_macros.h>
#include <core/version.h>
#include <windows.h>
#include <chrono>
#include <iostream>

namespace ballistic {

Error Application::create(const ApplicationCreateInfo& p_info)
{
    using enum Error;
    Error err;

    ballistic::log_write("%s v%s.stable.official - https://ballisticgames.ca", BALLISTIC_VERSION_NAME, BALLISTIC_VERSION_NUMBER);

    create_info = p_info;

    err = window.create(p_info.window_title, p_info.width, p_info.height);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = context_driver.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be called for other backends
    err = context_driver.surface_create(window.hwnd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be manually selected
    err = context_driver.physical_device_select();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    device_driver.context_driver = &context_driver;
    err = device_driver.initialize(context_driver.optimal_device_index, 3);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    renderer.device_driver = &device_driver;
    err = renderer.create(3);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    drivers::ImGuiDriverCreateInfo imgui_ci{};
    imgui_ci.hwnd = window.hwnd;
    imgui_ci.instance = context_driver.instance;
    imgui_ci.physical_device = device_driver.physical_device;
    imgui_ci.device = device_driver.device;
    imgui_ci.queue_family = context_driver.graphics_queue_family;
    imgui_ci.queue = device_driver.queue_families[context_driver.graphics_queue_family][0].queue;
    imgui_ci.color_format = device_driver.swapchain.format;
    imgui_ci.image_count = 3;
    imgui_ci.ini_path = p_info.ini_path;
    imgui_ci.enable_docking = wants_docking();
    err = imgui.create(imgui_ci);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    render_path = create_render_path();
    render_path->device_driver = &device_driver;
    render_path->imgui = &imgui;
    err = render_path->create_resources();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // dev_systems.create(&renderer, &device_driver);

    return Ok;
}

void Application::destroy()
{
    device_driver.device_wait_idle();
    if (render_path) {
        render_path->destroy_resources();
        delete render_path;
        render_path = nullptr;
    }
    imgui.destroy();
    renderer.destroy();
    device_driver.shutdown();
    context_driver.shutdown();
    window.destroy();
}

int Application::run()
{
    using enum Error;
    Error err;

    err = on_init();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, static_cast<int>(err));

    auto lastTime = std::chrono::steady_clock::now();

    while (!window.should_close()) {
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration<double>(now - lastTime).count();
        lastTime = now;

        window.poll_events();
        context_driver.surface_set_size(window.width, window.height);

        err = device_driver.update_swapchain();
        if (err != Ok) {
            continue;
        }

        imgui.new_frame();
        dev_systems.new_frame((float)delta);
        on_update((float)delta);
        imgui.render();

        renderer.begin_frame();

        GraphPass present_pass;
        present_pass.name = "present";
        present_pass.setup = [](RenderGraphBuilder& b) {
            b.write_image("backbuffer",
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        };
        present_pass.execute = [this](VkCommandBuffer cmd, RenderGraph& g) {
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
            imgui.record_commands(cmd);
            vkCmdEndRendering(cmd);
        };
        renderer.graph.add(&present_pass);

        render_path->build(renderer.graph);
        renderer.end_frame();
    }

    on_shutdown();
    destroy();
    return 0;
}

}