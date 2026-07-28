#pragma once
#include <editor/panel.h>

namespace ballistic {

struct Console : Panel
{
    const char* name() const override { return "Console"; }
    void draw_contents(EditorContext& ctx) override;
};

}