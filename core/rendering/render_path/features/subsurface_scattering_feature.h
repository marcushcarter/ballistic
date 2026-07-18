#pragma once
#include <core/rendering/render_path/features/feature.h>

namespace ballistic {

struct SubsurfaceScatteringFeature : Feature
{
    RenderGraph::Pass per_pixel_pass;
    // RenderGraph::Pass pass;
    
    RenderGraph::Pass sss_pass;
    RenderGraph::Pass composite_pass;

    Error create_resources() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}