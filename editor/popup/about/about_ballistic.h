#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct AboutBallisticPopup : Popup
{
    const char* name() const override { return "About Ballistic"; }
    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
};

}