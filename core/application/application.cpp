#include <core/application/application.h>
#include <core/version.h>
#include <windows.h>
#include <chrono>
#include <iostream>

namespace ballistic {

void Application::create(const ApplicationCreateInfo& p_info)
{
    std::cout << BALLISTIC_VERSION_NAME << " v" << BALLISTIC_VERSION_NUMBER << ".stable.official - https://ballisticgames.ca\n";

    create_info = p_info;
    window.create(p_info.window_title, p_info.width, p_info.height);

    drivers::ImGuiDriverCreateInfo imgui_create_info{};
    imgui_create_info.hwnd = window.hwnd;

    // imgui.create(imgui_create_info);
}

void Application::destroy()
{
    // imgui.destroy();
    window.destroy();
}

int Application::run()
{
    on_init();

    auto lastTime = std::chrono::steady_clock::now();
    auto startTime = lastTime;

    while (!window.should_close()) {
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration<double>(now - lastTime).count();
        lastTime = now;

        window.poll_events();
        // imgui.new_frame();

        on_update((float)delta);

        // imgui.render();
    }

    on_shutdown();
    destroy();
    return 0;
}

}