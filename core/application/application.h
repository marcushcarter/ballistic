#pragma once
#include <drivers/windows/window_driver_win32.h>
#include <drivers/vulkan/device_driver_vulkan.h>
#include <drivers/vulkan/context_driver_vulkan.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/dev_tools/dev_tools.h>
#include <core/rendering/renderer.h>
#include <core/log/error.h>
#include <string>

namespace ballistic {

struct RenderPath;

struct ApplicationCreateInfo
{
    std::string window_title;
    int width = 1280;
    int height = 720;
    const char* ini_path = nullptr;
};

struct Application
{
    drivers::WindowDriverWin32 window_driver;
    drivers::WindowDriverWin32::Window window;

    drivers::ContextDriverVulkan cd;
    drivers::DeviceDriverVulkan dd;
    Renderer renderer;
    RenderPath* render_path = nullptr;

    drivers::ImGuiDriver imgui;
    DevTools dev_tools;

    Error create(const ApplicationCreateInfo& p_create_info);
    void destroy();
    int run();

    virtual Error on_init() = 0;
    virtual void on_update(float p_dt) = 0;
    virtual void on_shutdown() = 0;
    
    virtual bool wants_docking() const { return false; }
    virtual RenderPath* create_render_path() = 0;

    virtual ~Application() = default;
};

}