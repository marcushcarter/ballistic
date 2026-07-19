#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/features/editor/imgui_feature.h>

namespace ballistic {

struct ProjectManagerRenderPath : RenderPath
{
    ImGuiFeature ui;

    ProjectManagerRenderPath() {
        ui.backbuffer_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        features.push_back(&ui);
    }
};

}