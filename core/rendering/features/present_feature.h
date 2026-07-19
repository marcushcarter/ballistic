#pragma once
#include <core/rendering/features/feature.h>

namespace ballistic {

struct PresentFeature : Feature
{    
    RenderGraph::Pass present_pass;
    
    drivers::DeviceDriverVulkan::Pipeline gamma_blit_pipeline;

    Error create_resources() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}