#pragma once
#include <core/rendering/render_path/render_path.h>

namespace ballistic {

struct EditorRenderPath : RenderPath
{
    GraphPass present_pass;
    
    Error create_resources() override;
    void build_present(RenderGraph& g) override;
};

}