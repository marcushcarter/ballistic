#pragma once
#include <drivers/windows/window_driver_windows.h>
#include <string>

namespace ballistic {

struct ApplicationCreateInfo
{
    std::wstring window_title;
    int width = 1280;
    int height = 720;
};

struct Application
{
    WindowDriverWindows window;
    ApplicationCreateInfo create_info;

    void create(const ApplicationCreateInfo& p_info);
    void destroy();

    int run();

    virtual void on_init() {}
    virtual void on_update(float p_dt) { (void)p_dt; }
    virtual void on_shutdown() {}

    virtual ~Application() = default;
};

}