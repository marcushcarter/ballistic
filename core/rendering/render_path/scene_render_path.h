#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/render_path/features/temp.h>
#include <core/rendering/render_path/features/visibility_feature.h>
#include <core/rendering/render_path/features/geometry_feature.h>
#include <core/rendering/render_path/features/ambient_occlusion_feature.h>
#include <core/rendering/render_path/features/deferred_lighting_feature.h>
#include <core/rendering/render_path/features/subsurface_scattering_feature.h>

namespace ballistic {

struct SceneRenderPath : RenderPath
{
    TempFeature temp;

    VisibilityFeature visibility;
    GeometryFeature geometry;
    AmbientOcclusionFeature ao;
    DeferredLightingFeature deferred_lighting;
    SubsurfaceScatteringFeature subsurface_scattering;
    
    // PostProcessFeature post_process;

    Error create_resources() override;
    void destroy_resources() override;
    void build(RenderGraph& g)  override final;
    virtual void build_present(RenderGraph& g) = 0;
};
    
}