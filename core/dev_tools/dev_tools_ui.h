#pragma once
#include <imgui.h>
#include <sal.h>
#include <cstdint>

namespace ballistic::ui {

void title(_Printf_format_string_ const char* p_fmt, ...);
void spacing();
void section_gap();

void property_row(const char* p_name, _Printf_format_string_ const char* p_fmt, ...);
void property_row_value_aligned(const char* p_name, _Printf_format_string_ const char* p_fmt, ...);

void cell_right(const char* p_text);
void cell_right_fmt(_Printf_format_string_ const char* p_fmt, ...);

void tri_right(ImU32 p_color);

const char* fmt_bytes(uint64_t b);

ImU32 rg_category_u32(const char* cat, float alpha = 1.0f);
ImU32 pct_col(float pct);

}