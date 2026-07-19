#pragma once
#include <core/dev_tools/dev_context.h>
#include <core/rendering/render_graph_profiler.h>
#include <cstdint>

namespace ballistic {

struct MemoryProfilerTransients
{
    static constexpr uint32_t MAX_TRACKED_HEAPS = 8;
    static constexpr uint32_t MAX_TRANSIENT_ROWS = 100;
    
    uint64_t frame_counter = 0;
    uint64_t peak_bytes = 0;

    float detailed_frag[MAX_TRACKED_HEAPS]{};
    bool detailed_valid = false;

    void draw(DevContext& ctx);
};

}