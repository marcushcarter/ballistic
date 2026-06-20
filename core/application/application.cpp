#include <core/application/application.h>
#include <core/error/error_macros.h>
#include <core/version.h>
#include <windows.h>
#include <chrono>
#include <iostream>

namespace ballistic {

Error Application::create(const ApplicationCreateInfo& p_info)
{
    using enum Error;
    Error err;

    std::cout << BALLISTIC_VERSION_NAME << " v" << BALLISTIC_VERSION_NUMBER << ".stable.official - https://ballisticgames.ca\n";
    
    create_info = p_info;

    err = window.create(p_info.window_title, p_info.width, p_info.height);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = vulkan_context.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be called for other backends
    err = vulkan_context.surface_create(window.hwnd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be manually selected
    err = vulkan_context.physical_device_select();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    vulkan_device.context_driver = &vulkan_context;
    err = vulkan_device.initialize(vulkan_context.optimal_device_index, 1);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // device.swapcain_create

    drivers::ImGuiDriverCreateInfo imgui_ci{};
    imgui_ci.hwnd = window.hwnd;
    imgui_ci.instance = vulkan_context.instance;

    // err = imgui.create(imgui_ci);
    // BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

void Application::destroy()
{
    // imgui.destroy();
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
        
        if (window.resize_requested) {
            if (window.pending_width != vulkan_context.surface.width ||
                window.pending_height != vulkan_context.surface.height) {
                vulkan_context.surface_set_size(window.pending_width, window.pending_height);
            }
            window.resize_requested = false;
        }

        // err = vulkan_device.check_resize();
        // BALLISTIC_ERR_FAIL_COND_V(err != Ok, static_cast<int>(err));
        
        // imgui.new_frame();
        on_update((float)delta);
        // imgui.render();
    }

    on_shutdown();
    destroy();
    return 0;
}

}