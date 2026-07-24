#pragma once
#include <core/dev_tools/dev_panel.h>
#include <core/dev_tools/memory_profiler/memory_profiler_transients.h>
#include <cstdint>
#include <vector>

namespace ballistic {

struct MemoryProfiler : DevPanel
{    
    uint64_t frame_counter = 0;
    uint64_t peak_bytes = 0;

    std::vector<float> detailed_frag;
    bool detailed_valid = false;

    MemoryProfilerTransients transients;

    const char* name() const override { return "Memory Profiler"; }
    void before_begin() override;
    void draw_contents(DevContext& ctx) override;
};

}