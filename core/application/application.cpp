#include <core/application/application.h>
#include <core/rendering/render_path/render_path.h>
#include <core/log/error_macros.h>
#include <core/version.h>
#include <windows.h>
#include <chrono>
#include <iostream>

namespace ballistic {

Error Application::create(const ApplicationCreateInfo& p_create_info)
{
    using enum Error;
    Error err;

    ballistic::log_write("%s v%s.stable.official - https://ballisticgames.ca", BALLISTIC_VERSION_NAME, BALLISTIC_VERSION_NUMBER);

    err = window_driver.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    window = window_driver.window_create(p_create_info.window_title, p_create_info.width, p_create_info.height);
    BALLISTIC_ERR_FAIL_COND_V(window.hwnd == nullptr, Failed);
    window_driver.window_bind(window);

    err = context_driver.full_initialize_windows(window.hwnd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = device_driver.initialize(context_driver, context_driver.optimal_device_index, 3);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = renderer.create(device_driver);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    drivers::ImGuiDriverCreateInfo imgui_ci{};
    imgui_ci.hwnd = window.hwnd;
    imgui_ci.instance = context_driver.instance;
    imgui_ci.physical_device = device_driver.physical_device;
    imgui_ci.device = device_driver.device;
    imgui_ci.queue_family = context_driver.graphics_queue_family;
    imgui_ci.queue = device_driver.queue_families[context_driver.graphics_queue_family][0].queue;
    imgui_ci.image_count = renderer.frame_count;
    imgui_ci.render_pass = renderer.swapchain_render_pass;
    imgui_ci.subpass = 0;
    imgui_ci.ini_path = p_create_info.ini_path;
    imgui_ci.enable_docking = wants_docking();
    err = imgui.create(imgui_ci);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    render_path = create_render_path();
    render_path->device_driver = &device_driver;
    render_path->imgui = &imgui;
    err = render_path->create_resources();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    dev_systems.create(renderer, device_driver);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    VkShaderModule vs = device_driver.shader_create({
        .stage = drivers::DeviceDriverVulkan::ShaderStage::Vertex,
        .glsl_source = R"(#version 450
            void main() { gl_Position = vec4(0); })",
        .name = "gbuffer_vs",
    });
    // ... build pipeline ...
    device_driver.shader_free(vs);

    return Ok;
}

void Application::destroy()
{
    device_driver.device_wait_idle();
    dev_systems.destroy();
    
    if (render_path) {
        render_path->destroy_resources();
        delete render_path;
        render_path = nullptr;
    }

    imgui.destroy();
    renderer.destroy();
    device_driver.shutdown();
    context_driver.shutdown();
    
    window_driver.window_free(window);
    window_driver.shutdown();
}

int Application::run()
{
    using enum Error;
    Error err;

    err = on_init();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, static_cast<int>(err));

    auto lastTime = std::chrono::steady_clock::now();

    while (!window_driver.window_should_close(window)) {
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration<double>(now - lastTime).count();
        lastTime = now;

        drivers::WindowDriverWin32::poll_events();

        context_driver.surface_set_size(window.width, window.height);

        err = device_driver.swapchain_update();
        if (err != Ok) {
            continue;
        }

        dev_systems.begin_frame();

        imgui.new_frame();
        on_update((float)delta);
        imgui.render();

        renderer.begin_frame();
        render_path->build(renderer.graph);
        renderer.end_frame();

        dev_systems.end_frame();
    }

    on_shutdown();
    destroy();
    return 0;
}

}