#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct AboutBallisticPopup : Popup
{
    const char* name() const override { return "About Ballistic"; }
    void draw_contents(EditorContext& ctx) override;
};

}