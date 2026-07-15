#pragma once
#include <core/dev_tools/dev_context.h>
#include <core/rendering/render_graph_profiler.h>

namespace ballistic {

struct ProfilerTimeline
{
    const RenderGraphProfiler::Timing* selected_pass = nullptr;
    const RenderGraphProfiler::Timing* selected_draw = nullptr;

    char sel_name[64] = {};
    bool follow = false;

    void draw(DevContext& ctx);
};

}