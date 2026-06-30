#pragma once
#include <drivers/windows/window_driver_win32.h>
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <drivers/vulkan/rendering_context_driver_vulkan.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/dev_tools/dev_systems.h>
#include <core/rendering/renderer.h>
#include <core/log/error.h>
#include <string>

namespace ballistic {

struct ApplicationCreateInfo
{
    std::string window_title;
    int width = 1280;
    int height = 720;
    const char* ini_path = nullptr;
};

struct Application
{
    ApplicationCreateInfo create_info;

    drivers::WindowDriverWin32 window;
    drivers::RenderingContextDriverVulkan context_driver;
    drivers::RenderingDeviceDriverVulkan device_driver;
    Renderer renderer;

    drivers::ImGuiDriver imgui;
    DevSystems dev_systems;

    Error create(const ApplicationCreateInfo& p_info);
    void destroy();
    int run();

    virtual Error on_init() = 0;
    virtual void on_update(float p_dt) = 0;
    virtual void on_shutdown() = 0;
    virtual bool wants_docking() const { return false; }
    virtual ~Application() = default;
};

}