#pragma once
#include <core/rendering/render_path.h>

namespace ballistic {

struct GameRenderPath : RenderPath
{
    Error create_resources() override;
    void destroy_resources() override;
    void build_present(RenderGraph& g) override;
};

}