#pragma once
#include <imgui.h>
#include <string>

namespace ballistic {

inline std::string color_to_hex(const ImVec4& c)
{
    auto q = [](float v) -> unsigned {
        float x = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
        return (unsigned)(x * 255.0f + 0.5f);
    };
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", q(c.x), q(c.y), q(c.z));
    return buf;
}

inline bool color_from_hex(std::string_view s, ImVec4& r_out)
{
    if (!s.empty() && s.front() == '#') s.remove_prefix(1);
    if (s.size() != 6 && s.size() != 8) return false;
    uint32_t v = 0;
    for (char ch : s) {
        v <<= 4;
        if (ch >= '0' && ch <= '9') v |= uint32_t(ch - '0');
        else if (ch >= 'a' && ch <= 'f') v |= uint32_t(ch - 'a' + 10);
        else if (ch >= 'A' && ch <= 'F') v |= uint32_t(ch - 'A' + 10);
        else return false;
    }
    if (s.size() == 6) {
        r_out = ImVec4(((v >> 16) & 0xFF) / 255.0f, ((v >> 8) & 0xFF) / 255.0f, (v & 0xFF) / 255.0f, 1.0f);
    } else {
        r_out = ImVec4(((v >> 24) & 0xFF) / 255.0f, ((v >> 16) & 0xFF) / 255.0f, ((v >> 8) & 0xFF) / 255.0f, (v & 0xFF) / 255.0f);
    }
    return true;
}

}