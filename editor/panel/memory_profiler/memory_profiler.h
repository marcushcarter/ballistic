#pragma once
#include <editor/panel/panel.h>
#include <editor/panel/memory_profiler/memory_profiler_transients.h>
#include <cstdint>
#include <vector>

namespace ballistic {

struct MemoryProfiler : Panel
{    
    uint64_t frame_counter = 0;
    uint64_t peak_bytes = 0;

    std::vector<float> detailed_frag;
    bool detailed_valid = false;

    MemoryProfilerTransients transients;

    const char* name() const override { return "Memory Profiler (NEW)"; }
    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
};

}