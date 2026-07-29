#pragma once
#include <editor/popup/popup.h>

namespace ballistic {

struct ExportPopup : Popup
{
    const char* name() const override { return "Export"; }
    void draw_contents(EditorContext& ctx) override;
};

}