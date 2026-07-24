#pragma once
#include <core/dev_tools/dev_context.h>
#include <core/rendering/render_graph_profiler.h>
#include <cstdint>

namespace ballistic {

struct MemoryProfilerTransients
{
    uint32_t max_rows = 100;

    void draw(DevContext& ctx);
};

}