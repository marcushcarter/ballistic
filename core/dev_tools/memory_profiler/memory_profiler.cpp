#include <core/dev_tools/memory_profiler/memory_profiler.h>
// #include <core/rendering/renderer.h>
#include <core/dev_tools/dev_tools_ui.h>
#include <imgui.h>
// #include <implot.h>

namespace ballistic {

void MemoryProfiler::before_begin()
{
    // ImGui::SetNextWindowSize(ImVec2(1100, 600), ImGuiCond_FirstUseEver);
}

void MemoryProfiler::draw_contents(DevContext& ctx)
{
    (void)ctx;
}

}