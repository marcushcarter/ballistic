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

    err = cd.full_initialize_windows(window.hwnd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = dd.initialize(cd, cd.optimal_device_index, 3);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = renderer.create(dd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    drivers::ImGuiDriverCreateInfo imgui_ci{};
    imgui_ci.hwnd = window.hwnd;
    imgui_ci.instance = cd.instance;
    imgui_ci.physical_device = dd.physical_device;
    imgui_ci.device = dd.device;
    imgui_ci.queue_family = cd.graphics_queue_family;
    imgui_ci.queue = dd.queue_families[cd.graphics_queue_family][0].queue;
    imgui_ci.image_count = renderer.frame_count;
    imgui_ci.render_pass = dd.swapchain.render_pass;
    imgui_ci.sampler = dd.default_sampler.sampler;
    imgui_ci.ini_path = p_create_info.ini_path;
    imgui_ci.enable_docking = wants_docking();
    err = imgui.create(imgui_ci);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    render_path = create_render_path();
    render_path->dd = &dd;
    render_path->imgui = &imgui;
    render_path->graph = &renderer.graph;
    err = render_path->create_resources();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    DevContext ctx{};
    ctx.renderer = &renderer;
    ctx.imgui = &imgui;

    dev_tools.create(ctx);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

void Application::destroy()
{
    dd.device_wait_idle();
    dev_tools.destroy();
    
    if (render_path) {
        render_path->destroy_resources();
        delete render_path;
        render_path = nullptr;
    }

    imgui.destroy();
    renderer.destroy();
    dd.shutdown();
    cd.shutdown();
    
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

        _apply_pending_render_path();

        drivers::WindowDriverWin32::poll_events();

        cd.surface_set_size(window.width, window.height);
        if (dd.swapchain_update() != Ok) continue;

        renderer.apply_pending_size();

        imgui.begin_frame(renderer.frame_number, renderer.frame_count, renderer.resize_epoch);
        renderer.begin_frame();
        
        render_path->build(renderer.graph);
        renderer.compile();

        on_update((float)delta);
        imgui.render();

        renderer.render_frame();
        
        renderer.end_frame();
        imgui.end_frame(renderer.frame_number);
    }

    on_shutdown();
    destroy();
    return 0;
}

void Application::render_path_request(RenderPath* p_next)
{
    if (pending_render_path) delete pending_render_path;
    pending_render_path = p_next;
}

void Application::_apply_pending_render_path()
{
    using enum Error;
    if (!pending_render_path) return;

    dd.device_wait_idle();
    
    renderer.resize_epoch++;

    if (render_path) {
        render_path->destroy_resources();
        delete render_path;
    }

    render_path = pending_render_path;
    pending_render_path = nullptr;

    render_path->dd = &dd;
    render_path->imgui = &imgui;
    render_path->graph = &renderer.graph;
    if (render_path->create_resources() != Ok)
        log_write("Application: render path create_resources failed.");
}

}