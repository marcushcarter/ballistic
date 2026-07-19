#pragma once
#include <core/dev_tools/dev_panel.h>
#include <core/dev_tools/memory_profiler/memory_profiler_transients.h>
#include <cstdint>

namespace ballistic {

struct MemoryProfiler : DevPanel
{
    static constexpr uint32_t MAX_TRACKED_HEAPS = 8;
    static constexpr uint32_t MAX_TRANSIENT_ROWS = 100;
    
    uint64_t frame_counter = 0;
    uint64_t peak_bytes = 0;

    float detailed_frag[MAX_TRACKED_HEAPS]{};
    bool detailed_valid = false;

    MemoryProfilerTransients transients;

    const char* name() const override { return "Memory Profiler"; }
    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}