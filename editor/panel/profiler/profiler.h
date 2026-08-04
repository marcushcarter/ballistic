#pragma once
#include <editor/panel/panel.h>
#include <editor/panel/profiler/profiler_timeline.h>
#include <editor/panel/profiler/profiler_distribution.h>
#include <editor/panel/profiler/profiler_resources.h>
#include <cstdint>

namespace ballistic {

struct ProfilerPanel : Panel
{
    const char* name() const override { return "GPU Profiler"; }

    ProfilerTimeline timeline;
    ProfilerDistribution distribution;
    ProfilerResources resources;

    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
};

}