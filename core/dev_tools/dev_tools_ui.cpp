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

void section_gap() {
    ImGui::Dummy(ImVec2(0, 2));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 2));
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

void cell_right(const char* p_text)
{
    float w = ImGui::CalcTextSize(p_text).x;
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > w) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - w));
    ImGui::TextUnformatted(p_text);
}

void cell_right_fmt(const char* p_fmt, ...)
{
    char buf[64];
    va_list ap; va_start(ap, p_fmt);
    vsnprintf(buf, sizeof(buf), p_fmt, ap);
    va_end(ap);
    cell_right(buf);
}

void tri_right(ImU32 p_color)
{
    ImVec2 c = ImGui::GetCursorScreenPos();
    float h = ImGui::GetFontSize();
    float s = h * 0.42f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddTriangleFilled(ImVec2(c.x, c.y + h * 0.28f), ImVec2(c.x, c.y + h * 0.72f), ImVec2(c.x + s, c.y + h * 0.50f), p_color);
    ImGui::Dummy(ImVec2(s + 4.0f, h));
    ImGui::SameLine(0, 4.0f);
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

ImU32 pct_col(float pct)
{
    if (pct < 0.75f) return IM_COL32( 88, 180, 120, 255);
    if (pct < 0.90f) return IM_COL32(210, 170,  70, 255);
    return IM_COL32(210,  90,  80, 255);
}

}