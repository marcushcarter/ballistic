#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

namespace ballistic {

namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderPath
{
    drivers::DeviceDriverVulkan* device_driver = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;

    RenderGraph::Pass temp_pass;

    virtual Error create_resources();
    virtual void destroy_resources();
    void build(RenderGraph& g);
    virtual void build_present(RenderGraph& g) = 0;
    
    virtual ~RenderPath() = default;
};
    
}