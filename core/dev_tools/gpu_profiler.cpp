#include <core/dev_tools/gpu_profiler.h>
#include <core/rendering/render_graph.h>
#include <drivers/vulkan/device_driver_vulkan.h>
// #include <core/log/log.h>
#include <imgui.h>

namespace ballistic {

void GpuProfiler::draw()
{
    if (!open) return;

    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    bool visible = ImGui::Begin("GPU Profiler", &open);

    if (visible) {
        
    }
    ImGui::End();
}

}