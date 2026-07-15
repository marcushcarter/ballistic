#include <core/dev_tools/profiler/profiler.h>
#include <core/rendering/renderer.h>
#include <imgui.h>

namespace ballistic {

void property_row_value_aligned(const char* name, const char* fmt, ...)
{
    ImGui::TextUnformatted(name);

    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    float value_width = ImGui::CalcTextSize(buffer).x;
    float right_edge = ImGui::GetContentRegionMax().x;

    ImGui::SameLine();
    ImGui::SetCursorPosX(right_edge - value_width);
    ImGui::TextUnformatted(buffer);
}


void Profiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
}

void Profiler::draw_contents(DevContext& ctx)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();

    const float right_width = avail.x * 0.20f;
    const float leftWidth = avail.x - right_width - ImGui::GetStyle().ItemSpacing.x;

    const float topHeight = avail.y / 3.0f;
    const float bottomHeight = avail.y - topHeight - ImGui::GetStyle().ItemSpacing.y;

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("TopLeft", ImVec2(leftWidth, topHeight), ImGuiChildFlags_Borders);
    ImGui::PopStyleVar();
    {
        timeline.draw(ctx);
    }
    ImGui::EndChild();

    ImGui::BeginChild("BottomLeft", ImVec2(leftWidth, bottomHeight), ImGuiChildFlags_Borders);
    {
        // if (ImGui::BeginTabBar("Tabs")) {
        //     if (ImGui::BeginTabItem("Resources")) {
        //         // Passes 6 active / 6 total.
        //         // Images: 4. Buffers: 0
        //         // resources graph
        //         // Images.
        //         // Name | Kind | Format | Life | Produces | R | W
        //         // Buffers.
        //         // Name | Kind | Size | Mem | Life | Producer | R/W

        //         ImGui::EndTabItem();
        //     }
        //     if (ImGui::BeginTabItem("Materials")) {
        //         ImGui::EndTabItem();
        //     }
        //     ImGui::EndTabBar();
        // }

        if (timeline.selected_pass != nullptr) {
            ImGui::Text("Selected Pass");
            ImGui::Text("Name: %s", timeline.selected_pass->name);
            ImGui::Text("Category: %s", timeline.selected_pass->category);
            ImGui::Text("GPU Time: %.3f ms", timeline.selected_pass->gpu_ms);
            ImGui::Text("Raw Time: %.3f ms", timeline.selected_pass->raw_ms);
            ImGui::Text("Draw Count: %u", timeline.selected_pass->draw_count);
        }
    }
    ImGui::EndChild();

    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();

    const float header_height = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("RightHeader", ImVec2(right_width, header_height), ImGuiChildFlags_None);
    auto& profiler = ctx.renderer->graph.profiler;
    ImGui::BeginDisabled(!profiler.supported);
    ImGui::Checkbox("Enable Profiling", &profiler.enabled);
    ImGui::EndDisabled();
    ImGui::EndChild();

    ImGui::BeginChild("Right", ImVec2(right_width, avail.y - header_height - ImGui::GetStyle().ItemSpacing.y), ImGuiChildFlags_Borders);
    {

        if (timeline.selected_draw != nullptr) {
            ImGui::Text("Name: %s", timeline.selected_draw->name ? timeline.selected_draw->name : "Unnamed");
            ImGui::Text("Type: %s", timeline.selected_draw->type);
            ImGui::Text("GPU Time: %.3f ms", timeline.selected_draw->gpu_ms);
            ImGui::Text("Raw Time: %.3f ms", timeline.selected_draw->raw_ms);
        }

        property_row_value_aligned("Pan Area", "ALT + Mouse");
        property_row_value_aligned("Zoom", "ScrollY");
        property_row_value_aligned("Zoom Area", "Mouse Drag");
        property_row_value_aligned("Zoom Out", "Double Click");
        property_row_value_aligned("Zoom In", "ALT + Double Click");
        property_row_value_aligned("Frame Pass", "F");
        property_row_value_aligned("Cancel Drag", "Escape");
        
    }
    ImGui::EndChild();

    ImGui::EndGroup();
}

}