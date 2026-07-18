#pragma once
#include <core/rendering/render_graph.h>
#include <core/log/error.h>

#include <core/rendering/render_path/features/temp.h>
// #include <core/rendering/render_path/features/post_processing_feature.h>

#include <core/rendering/render_path/features/visibility_feature.h>
#include <core/rendering/render_path/features/geometry_feature.h>
#include <core/rendering/render_path/features/ambient_occlusion_feature.h>
#include <core/rendering/render_path/features/deferred_lighting_feature.h>
#include <core/rendering/render_path/features/subsurface_scattering_feature.h>

namespace ballistic {

namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderPath
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;

    TempFeature temp;

    VisibilityFeature visibility;
    GeometryFeature geometry;
    AmbientOcclusionFeature ao;
    DeferredLightingFeature deferred_lighting;
    SubsurfaceScatteringFeature subsurface_scattering;
    
    // PostProcessFeature post_process;

    virtual Error create_resources();
    virtual void destroy_resources();
    void build(RenderGraph& g);
    virtual void build_present(RenderGraph& g) = 0;
    
    virtual ~RenderPath() = default;
};
    
}