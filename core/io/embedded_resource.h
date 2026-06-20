#pragma once
#include <windows.h>
#include <string>

namespace ballistic {

struct EmbeddedResource
{
    static HICON load_icon(const std::wstring& p_resource_name);
    static HFONT load_font(const std::wstring& p_resource_name);
};

}