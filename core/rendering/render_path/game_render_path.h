#pragma once
#include <core/rendering/render_path/render_path.h>

namespace ballistic {

struct GameRenderPath : RenderPath
{
    GraphPass present_pass;
    // VkPipeline blit_pipeline = VK_NULL_HANDLE;
    
    Error create_resources() override;
    void destroy_resources() override;
    void build_present(RenderGraph& g) override;
};

}