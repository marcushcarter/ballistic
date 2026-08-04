#pragma once
#include <editor/panel/panel.h>

namespace ballistic {

struct ConsolePanel : Panel
{
    const char* name() const override { return "Console"; }
    void draw_contents(EditorContext& ctx) override;
};

}