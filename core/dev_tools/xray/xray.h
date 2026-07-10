#pragma once
#include <core/dev_tools/dev_panel.h>
#include <cstdint>

namespace ballistic {

struct Xray : DevPanel
{
    const char* name() const override { return "Xray"; }
    bool show_in_editor() const override { return false; }

    uint64_t selected_name_id = 0;

    int push_style() override;
    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}