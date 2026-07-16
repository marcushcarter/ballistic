#include <core/dev_tools/profiler/profiler.h>
#include <core/rendering/renderer.h>
#include <core/dev_tools/dev_tools_ui.h>
#include <imgui.h>

namespace ballistic {

void Profiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
}

void Profiler::draw_contents(DevContext& ctx)
{
    auto& profiler = ctx.renderer->graph.profiler;

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
    
    ImGui::BeginChild("Right", ImVec2(right_width, avail.y), ImGuiChildFlags_Borders);
    {        
        ImGui::BeginDisabled(!profiler.supported);
        ImGui::Checkbox("Enable Profiling", &profiler.enabled);
        ImGui::EndDisabled();
        
        ImGui::TextDisabled("%s", profiler.frozen ? "Frozen" : "Live");

        ui::title("Legend");
        ui::property_row_value_aligned("Pan Area", "ALT + Mouse");
        ui::property_row_value_aligned("Navigate", "Mouse Scroll");
        ui::property_row_value_aligned("Zoom Area", "Mouse Drag");
        ui::property_row_value_aligned("Zoom Out", "Double Click");
        ui::property_row_value_aligned("Frame Pass", "F");
        ui::property_row_value_aligned("Record", "Space");
        ui::property_row_value_aligned("Resume", "Esc");
        ui::spacing();

        if (timeline.selected_draw != nullptr) {
            ImGui::Text("Name: %s", timeline.selected_draw->name ? timeline.selected_draw->name : "Unnamed");
            ImGui::Text("Type: %s", timeline.selected_draw->type);
            ImGui::Text("GPU Time: %.3f ms", timeline.selected_draw->gpu_ms);
            ImGui::Text("Raw Time: %.3f ms", timeline.selected_draw->raw_ms);
        }
    }
    ImGui::EndChild();
}

}