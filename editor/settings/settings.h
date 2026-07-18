#pragma once
#include <editor/panel.h>

namespace ballistic {

struct Settings : Panel
{
    const char* name() const override { return "Editor Settings"; }
    void draw_contents(EditorContext& ctx) override;
};

}