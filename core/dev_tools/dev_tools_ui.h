#pragma once
#include <sal.h>

namespace ballistic::ui {

void title(_Printf_format_string_ const char* p_fmt, ...);
void spacing();

void property_row(const char* p_name, _Printf_format_string_ const char* p_fmt, ...);
void property_row_value_aligned(const char* p_name, _Printf_format_string_ const char* p_fmt, ...);

}