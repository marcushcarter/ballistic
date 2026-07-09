#pragma once
#include <cstdint>

namespace ballistic {

struct RenderGraph;

struct GpuProfiler
{
    bool open = false;
    void draw(RenderGraph& graph);
};

}