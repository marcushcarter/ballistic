#pragma once
#include <core/rendering/features/feature.h>

namespace ballistic {

struct GeometryFeature : Feature
{    
    RenderGraph::Pass geometry_pass;

    Error create_resources() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}