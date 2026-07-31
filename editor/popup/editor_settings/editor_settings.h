#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct EditorSettingsPopup : Popup
{
    const char* name() const override { return "Editor Settings"; }
    ImVec2 initial_size() const override { return ImVec2(500, 300); }
    void draw_contents(EditorContext& ctx) override;
};

}