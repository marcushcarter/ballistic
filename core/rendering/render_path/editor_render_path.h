#pragma once
#include <core/rendering/render_path/scene_render_path.h>
#include <core/rendering/features/editor/imgui_feature.h>
#include <core/rendering/features/editor/screenshot_feature.h>

namespace ballistic {

struct EditorRenderPath : SceneRenderPath
{
    ImGuiFeature ui;
    ScreenshotFeature screenshot;    

    EditorRenderPath() {
        ui.backbuffer_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        ui.sampled_image = "Out_Color";
        features.push_back(&ui);
        features.push_back(&screenshot);
    }
};

}