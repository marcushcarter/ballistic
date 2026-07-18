#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

namespace ballistic {

namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderPath
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;
    
    virtual Error create_resources() { return Error::Ok; }
    virtual void destroy_resources() {}
    virtual void build(RenderGraph& g) = 0;

    virtual ~RenderPath() = default;
};
    
}