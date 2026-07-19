#pragma once

namespace ballistic {

struct RenderGraph;
namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderContext
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;
};

}