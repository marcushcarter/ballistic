#include <core/application/application.h>
#include <windows.h>
#include <chrono>

namespace ballistic {

void Application::create(const ApplicationCreateInfo& p_info)
{
    create_info = p_info;
    window.create(p_info.window_title, p_info.width, p_info.height);
}

void Application::destroy()
{
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
        on_update((float)delta);
    }

    on_shutdown();
    destroy();
    return 0;
}

}