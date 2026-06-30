#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

namespace ballistic {

namespace drivers { struct RenderingDeviceDriverVulkan; struct ImGuiDriver; }

struct RenderPath
{
    drivers::RenderingDeviceDriverVulkan* device_driver = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;

    virtual Error create_resources();
    virtual void destroy_resources();
    void build(RenderGraph& g);
    virtual void build_present(RenderGraph& g) = 0;
    
    virtual ~RenderPath() = default;
};
    
}