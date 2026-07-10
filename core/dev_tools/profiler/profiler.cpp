#include <core/dev_tools/profiler/profiler.h>
#include <core/rendering/renderer.h>
#include <core/log/log.h>
#include <imgui.h>

namespace ballistic {

void Profiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(750, 400), ImGuiCond_FirstUseEver);
}

void Profiler::draw_contents(DevContext& ctx)
{
    (void)ctx;
}

}