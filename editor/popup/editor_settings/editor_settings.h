#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct EditorSettingsPopup : Popup
{
    const char* name() const override { return "Editor Settings"; }
    void draw_contents(EditorContext& ctx) override;
};

}