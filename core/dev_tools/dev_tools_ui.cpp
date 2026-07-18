#include <core/dev_tools/dev_tools_ui.h>
#include <cstdarg>
#include <cstdio>
#include <cstdint>

namespace ballistic::ui {

void title(const char* p_fmt, ...)
{
    va_list args;
    va_start(args, p_fmt);
    ImGui::TextColoredV(ImGui::GetStyleColorVec4(ImGuiCol_TextLink), p_fmt, args);
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

ImU32 rg_category_u32(const char* cat, float alpha)
{
    ImVec4 c(0.70f, 0.70f, 0.70f, alpha);
    if (cat && cat[0]) {
        uint64_t h = 1469598103934665603ull;
        for (const char* p = cat; *p; ++p) { h ^= (uint8_t)*p; h *= 1099511628211ull; }
        c = (ImVec4)ImColor::HSV((float)(h % 360) / 360.0f, 0.55f, 0.95f);
        c.w = alpha;
    }
    return ImGui::GetColorU32(c);
}

}