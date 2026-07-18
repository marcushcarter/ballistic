#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/render_path/features/screenshot_feature.h>

namespace ballistic {

struct ProjectManagerRenderPath : RenderPath
{
    RenderGraph::Pass ui_pass;
    
    Error create_resources() override;
    void build(RenderGraph& g) override;
};

}