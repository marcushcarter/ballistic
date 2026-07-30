#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct NewProjectPopup : Popup
{
    const char* name() const override { return "New Project"; }
    void draw_contents(EditorContext& ctx) override;
};

}