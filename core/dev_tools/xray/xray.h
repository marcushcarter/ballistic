#pragma once
#include <core/dev_tools/dev_panel.h>
#include <cstdint>

namespace ballistic {

struct Xray : DevPanel
{
    uint64_t selected_name_id = 0;

    const char* name() const override { return "Xray"; }
    bool show_in_editor() const override { return false; }
    ImGuiWindowFlags window_flags() const override { return ImGuiWindowFlags_NoDocking; }

    int push_style() override;
    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}