#pragma once
#include <core/rendering/render_path/scene_render_path.h>
#include <core/rendering/features/present_feature.h>

namespace ballistic {

struct GameRenderPath : SceneRenderPath
{
    PresentFeature present; 

    GameRenderPath() {
        features.push_back(&present);
    }
};

}