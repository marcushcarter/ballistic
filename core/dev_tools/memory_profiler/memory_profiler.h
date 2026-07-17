#pragma once
#include <core/dev_tools/dev_panel.h>
#include <cstdint>

namespace ballistic {

struct MemoryProfiler : DevPanel
{
    const char* name() const override { return "Memory Profiler"; }

    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}