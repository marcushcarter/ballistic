#pragma once
#include <drivers/windows/window_driver_win32.h>
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <drivers/vulkan/rendering_context_driver_vulkan.h>
#include <drivers/imgui/imgui_driver.h>
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
    drivers::WindowDriverWin32 window;
    drivers::RenderingContextDriverVulkan vulkan_context;
    drivers::RenderingDeviceDriverVulkan vulkan_device;
    drivers::ImGuiDriver imgui;
    
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