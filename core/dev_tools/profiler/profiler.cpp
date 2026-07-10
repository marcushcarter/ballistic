#include <core/dev_tools/profiler/profiler.h>
#include <imgui.h>

namespace ballistic {

void Profiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
}

void Profiler::draw_contents(DevContext& ctx)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();

    const float rightWidth = avail.x * 0.20f;
    const float leftWidth = avail.x - rightWidth - ImGui::GetStyle().ItemSpacing.x;

    const float topHeight = avail.y / 3.0f;
    const float bottomHeight = avail.y - topHeight - ImGui::GetStyle().ItemSpacing.y;

    ImGui::BeginGroup();

    ImGui::BeginChild("TopLeft", ImVec2(leftWidth, topHeight), ImGuiChildFlags_Borders);
    {
        timeline.draw(ctx);
    }
    ImGui::EndChild();

    ImGui::BeginChild("BottomLeft", ImVec2(leftWidth, bottomHeight), ImGuiChildFlags_Borders);
    {
        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("Resources")) {
                // ...
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Materials")) {
                // ...
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();

    ImGui::EndGroup();
    ImGui::SameLine();

    ImGui::BeginChild("Right", ImVec2(rightWidth, avail.y), ImGuiChildFlags_Borders);
    {
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();

    }
    ImGui::EndChild();
}

}