#pragma once
#include <core/rendering/render_path/scene_render_path.h>
#include <core/rendering/features/present_feature.h>
#include <core/rendering/features/editor/imgui_feature.h>

namespace ballistic {

struct GameRenderPath : SceneRenderPath
{
    PresentFeature present;
    ImGuiFeature ui;    

    GameRenderPath() {
        ui.sampled_image = "Out_Color";
        features.push_back(&present);
        features.push_back(&ui);
    }
};

}