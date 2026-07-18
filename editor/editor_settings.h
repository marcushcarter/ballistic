#pragma once
#include <imgui.h>
#include <string>
#include <iterator>
#include <cstdio>

namespace ballistic {

struct EditorSettings
{
    // ---- THEME -----

    struct Theme {
        ImVec4 base { 0.12f, 0.12f, 0.14f, 1.0f };
        ImVec4 accent { 0.66f, 0.30f, 0.76f, 1.0f };
        bool use_system_accent = false;
        int preset = -1;

        void apply() const;
    };

    Theme theme;
};

struct ThemePreset { const char* name; ImVec4 base, accent; };

inline constexpr ThemePreset THEME_PRESETS[] = {
    { "Dark",     { 0.12f, 0.12f, 0.14f, 1.0f }, { 0.66f, 0.30f, 0.76f, 1.0f } },
    { "Graphite", { 0.14f, 0.14f, 0.15f, 1.0f }, { 0.35f, 0.55f, 0.85f, 1.0f } },
    { "Midnight", { 0.08f, 0.09f, 0.13f, 1.0f }, { 0.30f, 0.70f, 0.85f, 1.0f } },
};

inline const char* theme_preset_name(int i)
{
    return (i < 0 || i >= (int)std::size(THEME_PRESETS)) ? "Custom" : THEME_PRESETS[i].name;
}

inline int theme_preset_index(std::string_view n)
{
    for (int i = 0; i < (int)std::size(THEME_PRESETS); ++i) if (n == THEME_PRESETS[i].name) return i;
    return -1;
}

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