#include <core/dev_tools/dev_tools_ui.h>
#include <imgui.h>
#include <cstdarg>
#include <cstdio>

namespace ballistic::ui {

void title(const char* p_fmt, ...)
{
    va_list args;
    va_start(args, p_fmt);
    ImGui::TextColoredV(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), p_fmt, args);
    // ImGui::TextV(p_fmt, args);
    va_end(args);
}
    
void spacing()
{
    ImGui::Spacing();
    ImGui::Spacing();
}

void property_row(const char* p_name, const char* p_fmt, ...)
{
    constexpr float tab_width = 200.0f;
    ImGui::TextUnformatted(p_name);
    ImGui::SameLine(tab_width);
    va_list args;
    va_start(args, p_fmt);
    ImGui::TextV(p_fmt, args);
    va_end(args);
}

void property_row_value_aligned(const char* p_name, const char* p_fmt, ...)
{
    ImGui::TextUnformatted(p_name);

    char buffer[256];

    va_list args;
    va_start(args, p_fmt);
    vsnprintf(buffer, sizeof(buffer), p_fmt, args);
    va_end(args);

    float value_width = ImGui::CalcTextSize(buffer).x;
    float right_edge = ImGui::GetContentRegionMax().x;

    ImGui::SameLine();
    ImGui::SetCursorPosX(right_edge - value_width);
    ImGui::TextUnformatted(buffer);
}

}