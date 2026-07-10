#pragma once
#include <core/dev_tools/dev_panel.h>
#include <cstdint>

namespace ballistic {

struct ClaudeProfiler : DevPanel
{
    const char* name() const override { return "Claude Profiler"; }

    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}