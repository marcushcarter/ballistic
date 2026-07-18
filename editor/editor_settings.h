#pragma once
#include <imgui.h>
#include <string>
#include <map>
#include <iterator>
#include <cstdio>

namespace ballistic {

struct Theme {
    ImVec4 base { 0.12f, 0.12f, 0.14f, 1.0f };
    ImVec4 accent { 0.66f, 0.30f, 0.76f, 1.0f };
    bool use_system_accent = false;
    int preset = -1;

    void apply() const;
    
    struct ThemePreset { const char* name; ImVec4 base, accent; };

    static inline constexpr ThemePreset THEME_PRESETS[] = {
        { "Dark",     { 0.12f, 0.12f, 0.14f, 1.0f }, { 0.66f, 0.30f, 0.76f, 1.0f } },
        { "Graphite", { 0.14f, 0.14f, 0.15f, 1.0f }, { 0.35f, 0.55f, 0.85f, 1.0f } },
        { "Midnight", { 0.08f, 0.09f, 0.13f, 1.0f }, { 0.30f, 0.70f, 0.85f, 1.0f } },
    };
    
    static const char* theme_preset_name(int i);
    static int theme_preset_index(std::string_view n);
};

struct EditorSettings
{
    Theme theme;

    std::map<std::string, bool> panel_open;
    bool profiler_enabled = false;

    void load();
    void save() const;
};

}