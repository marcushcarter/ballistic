#pragma once
#include <core/dev_tools/dev_panel.h>
#include <core/dev_tools/profiler/profiler_timeline.h>
#include <cstdint>

namespace ballistic {

struct Profiler : DevPanel
{
    const char* name() const override { return "GPU Profiler"; }

    ProfilerTimeline timeline;

    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}