#pragma once
#include <editor/panel.h>
#include <cstdint>

namespace ballistic {

struct Viewport : Panel
{
    uint64_t selected_name_id = 0;

    const char* name() const override { return "Viewport"; }
    int push_style() override;
    void draw_contents(EditorContext& ctx) override;
};

}