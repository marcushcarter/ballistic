#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/render_path/features/screenshot_feature.h>

namespace ballistic {

struct EditorRenderPath : RenderPath
{
    RenderGraph::Pass editor_ui_pass;
    ScreenshotFeature screenshot;
    
    Error create_resources() override;
    void destroy_resources() override;
    void build_present(RenderGraph& g) override;
};

}