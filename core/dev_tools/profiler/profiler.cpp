#include <core/dev_tools/profiler/profiler.h>
#include <core/rendering/renderer.h>
#include <core/dev_tools/dev_tools_ui.h>
#include <imgui.h>
#include <implot.h>

namespace ballistic {

void Profiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(1100, 600), ImGuiCond_FirstUseEver);
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
        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("Resources")) {

                resources.draw(ctx);
                
                if (timeline.selected_pass != nullptr) {
                    ImGui::Text("Selected Pass");
                    ImGui::Text("Name: %s", timeline.selected_pass->name);
                    ImGui::Text("Category: %s", timeline.selected_pass->category);
                    ImGui::Text("GPU Time: %.3f ms", timeline.selected_pass->gpu_ms);
                    ImGui::Text("Raw Time: %.3f ms", timeline.selected_pass->raw_ms);
                    ImGui::Text("Draw Count: %u", timeline.selected_pass->draw_count);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Materials")) {
                const float col_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

                ImGui::BeginChild("MatLeft", ImVec2(col_w, 0), ImGuiChildFlags_Borders);
                {

                }
                ImGui::EndChild();

                ImGui::SameLine();
                ImGui::BeginChild("MatMid", ImVec2(col_w, 0), ImGuiChildFlags_Borders);
                {

                }
                ImGui::EndChild();

                ImGui::SameLine();
                ImGui::BeginChild("MatRight", ImVec2(0, 0), ImGuiChildFlags_Borders);
                {

                }
                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
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

        ui::title("Legend");
        ui::property_row_value_aligned("Pan Area", "ALT + Mouse");
        ui::property_row_value_aligned("Navigate", "Mouse Scroll");
        ui::property_row_value_aligned("Zoom Area", "Mouse Drag");
        ui::property_row_value_aligned("Zoom Out", "Double Click");
        ui::property_row_value_aligned("Frame Pass", "F");
        ui::property_row_value_aligned("Record", "Space");
        ui::property_row_value_aligned("Resume", "ESC");
        ui::spacing();

        distribution.draw(ctx, timeline.selected_draw, profiler.frozen);
    }
    ImGui::EndChild();
}

}