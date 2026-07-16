#include <core/dev_tools/profiler/profiler_resources.h>

#include <core/dev_tools/profiler/profiler_distribution.h>
#include <core/rendering/renderer.h>
#include <core/dev_tools/dev_tools_ui.h>
#include <imgui.h>
#include <implot.h>
#include <algorithm>
#include <cstdio>

namespace ballistic {

void ProfilerResources::draw(DevContext& ctx)
{
    (void)ctx;

    // Passes 6 active / 6 total.
    // Images: 4. Buffers: 0
    // resources graph
    // Images.
    // Name | Kind | Format | Life | Produces | R | W
    // Buffers.
    // Name | Kind | Size | Mem | Life | Producer | R/W

    ImGui::TextDisabled("Resources");
}

}