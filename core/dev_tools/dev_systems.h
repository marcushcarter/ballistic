#pragma once
#include <core/dev_tools/renderbuffer_xray.h>
#include <core/dev_tools/debug_console.h>
#include <core/dev_tools/imgui_texture_cache.h>
#include <core/log/error.h>

namespace ballistic {

struct Renderer;
namespace drivers { struct DeviceDriverVulkan; }

struct DevSystems
{
    ImGuiTextureCache texture_cache;

    RenderBufferXray renderbuffer_xray;
    DebugConsole debug_console;

    Renderer* renderer = nullptr;
    drivers::DeviceDriverVulkan* device_driver = nullptr;

    Error create(Renderer& r_renderer, drivers::DeviceDriverVulkan& r_device_driver);
    void destroy();

    void begin_frame();
    void end_frame();
};

}