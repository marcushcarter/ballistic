#pragma once
#include <editor/panel.h>

namespace ballistic {

struct Debugger : Panel
{
    const char* name() const override { return "Debugger"; }

    void draw_contents(EditorContext& ctx) override;
};

}