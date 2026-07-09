#pragma once
#include <cstdint>

namespace ballistic {

struct RenderGraph;
struct ImGuiTextureCache;

struct RenderBufferXray
{
    bool open = false;
    uint64_t selected_name_id = 0;
    void draw(RenderGraph& graph, ImGuiTextureCache& cache);
};

}