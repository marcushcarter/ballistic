#include <core/io/embedded_resource.h>

namespace ballistic {

HICON EmbeddedResource::load_icon(const std::wstring& p_resource_name) {
    return LoadIconW(GetModuleHandleW(nullptr), p_resource_name.c_str());
}

HFONT EmbeddedResource::load_font(const std::wstring& p_resource_name)
{
    (void)p_resource_name;
    return nullptr;
}

}