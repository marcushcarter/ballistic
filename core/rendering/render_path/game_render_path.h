#pragma once
#include <core/rendering/render_path/render_path.h>
#include <drivers/vulkan/device_driver_vulkan.h>

namespace ballistic {

struct GameRenderPath : RenderPath
{
    RenderGraph::Pass present_pass;
    drivers::DeviceDriverVulkan::Pipeline gamma_blit_pipeline;
    
    Error create_resources() override;
    void destroy_resources() override;
    void build_present(RenderGraph& g) override;
};

}