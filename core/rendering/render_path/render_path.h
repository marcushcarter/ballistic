#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

#include <core/rendering/render_path/features/temp.h>

namespace ballistic {

namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderPath
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;

    TempFeature temp;

    virtual Error create_resources();
    virtual void destroy_resources();
    void build(RenderGraph& g);
    virtual void build_present(RenderGraph& g) = 0;
    
    virtual ~RenderPath() = default;
};
    
}