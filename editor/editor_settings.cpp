#include <editor/editor_settings.h>
#include <drivers/windows/window_driver_win32.h>
#include <core/io/path.h>
#include <core/math/color.h>
#include <fstream>
#include <cstdlib>
#include <cstdint>

namespace ballistic {

static ImVec4 shade(const ImVec4& c, float f) { return { c.x*f, c.y*f, c.z*f, c.w }; }
static ImVec4 fade(const ImVec4& c, float a) { return { c.x, c.y, c.z, a }; }
static ImVec4 mix(const ImVec4& a, const ImVec4& b, float t) { return { a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t, a.w }; }
static float luminance(const ImVec4& c) { return 0.2126f*c.x + 0.7152f*c.y + 0.0722f*c.z; }

void Theme::apply() const
{
    ImVec4 acc = accent;
    if (use_system_accent) {
        float r, g, b;
        if (drivers::WindowDriverWin32::system_accent_color(r, g, b)) acc = ImVec4(r, g, b, 1.0f);
    }

    ImGui::StyleColorsDark();
    ImVec4* c = ImGui::GetStyle().Colors;

    const ImVec4 text = (luminance(base) < 0.5f) ? ImVec4(0.92f, 0.92f, 0.94f, 1.0f) : ImVec4(0.10f, 0.10f, 0.12f, 1.0f);

    c[ImGuiCol_Text]                       = text;
    c[ImGuiCol_TextDisabled]               = mix(text, base, 0.55f);
    c[ImGuiCol_TextLink]                   = acc;
    c[ImGuiCol_TextSelectedBg]             = fade(acc, 0.35f);
    c[ImGuiCol_UnsavedMarker]              = text;

    // Surfaces
    c[ImGuiCol_WindowBg]                   = base;
    c[ImGuiCol_ChildBg]                    = shade(base, 0.92f);
    c[ImGuiCol_PopupBg]                    = shade(base, 0.85f);
    c[ImGuiCol_MenuBarBg]                  = shade(base, 0.90f);
    c[ImGuiCol_TitleBg]                    = shade(base, 0.80f);
    c[ImGuiCol_TitleBgActive]              = shade(base, 0.90f);
    c[ImGuiCol_TitleBgCollapsed]           = shade(base, 0.70f);
    c[ImGuiCol_DockingEmptyBg]             = shade(base, 0.70f);
    c[ImGuiCol_ModalWindowDimBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.45f);
    c[ImGuiCol_NavWindowingDimBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);

    // Frames (inputs, checkboxes, combos)
    c[ImGuiCol_FrameBg]                    = mix(shade(base, 1.25f), acc, 0.06f);
    c[ImGuiCol_FrameBgHovered]             = mix(shade(base, 1.45f), acc, 0.18f);
    c[ImGuiCol_FrameBgActive]              = mix(shade(base, 1.60f), acc, 0.30f);
    c[ImGuiCol_CheckboxSelectedBg]         = mix(shade(base, 1.60f), acc, 0.25f);
    c[ImGuiCol_InputTextCursor]            = text;

    // Lines
    c[ImGuiCol_Border]                     = shade(base, 1.80f);
    c[ImGuiCol_BorderShadow]               = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_Separator]                  = shade(base, 1.80f);
    c[ImGuiCol_SeparatorHovered]           = fade(acc, 0.70f);
    c[ImGuiCol_SeparatorActive]            = acc;
    c[ImGuiCol_TreeLines]                  = shade(base, 1.60f);

    // Window resize grip
    c[ImGuiCol_ResizeGrip]                 = fade(acc, 0.20f);
    c[ImGuiCol_ResizeGripHovered]          = fade(acc, 0.50f);
    c[ImGuiCol_ResizeGripActive]           = acc;

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]                = shade(base, 0.80f);
    c[ImGuiCol_ScrollbarGrab]              = shade(base, 1.60f);
    c[ImGuiCol_ScrollbarGrabHovered]       = fade(acc, 0.50f);
    c[ImGuiCol_ScrollbarGrabActive]        = acc;

    // Interactive
    c[ImGuiCol_Header]                     = fade(acc, 0.35f);
    c[ImGuiCol_HeaderHovered]              = fade(acc, 0.55f);
    c[ImGuiCol_HeaderActive]               = acc;
    c[ImGuiCol_Button]                     = fade(acc, 0.30f);
    c[ImGuiCol_ButtonHovered]              = fade(acc, 0.50f);
    c[ImGuiCol_ButtonActive]               = acc;
    c[ImGuiCol_CheckMark]                  = acc;
    c[ImGuiCol_SliderGrab]                 = acc;
    c[ImGuiCol_SliderGrabActive]           = acc;

    // Tabs
    c[ImGuiCol_Tab]                        = shade(base, 0.90f);
    c[ImGuiCol_TabHovered]                 = fade(acc, 0.60f);
    c[ImGuiCol_TabSelected]                = fade(acc, 0.80f);
    c[ImGuiCol_TabSelectedOverline]        = acc;
    c[ImGuiCol_TabDimmed]                  = shade(base, 0.85f);
    c[ImGuiCol_TabDimmedSelected]          = shade(base, 1.05f);
    c[ImGuiCol_TabDimmedSelectedOverline]  = fade(acc, 0.40f);

    // Docking / drag-drop / nav
    c[ImGuiCol_DockingPreview]             = fade(acc, 0.60f);
    c[ImGuiCol_DragDropTarget]             = acc;
    c[ImGuiCol_DragDropTargetBg]           = fade(acc, 0.15f);
    c[ImGuiCol_NavCursor]                  = acc;
    c[ImGuiCol_NavWindowingHighlight]      = fade(acc, 0.70f);

    // Plots
    c[ImGuiCol_PlotLines]                  = mix(text, base, 0.35f);
    c[ImGuiCol_PlotLinesHovered]           = acc;
    c[ImGuiCol_PlotHistogram]              = fade(acc, 0.75f);
    c[ImGuiCol_PlotHistogramHovered]       = acc;

    // Tables
    c[ImGuiCol_TableHeaderBg]              = shade(base, 1.20f);
    c[ImGuiCol_TableBorderStrong]          = shade(base, 1.90f);
    c[ImGuiCol_TableBorderLight]           = shade(base, 1.50f);
    c[ImGuiCol_TableRowBg]                 = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt]              = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
}
    
const char* Theme::theme_preset_name(int i)
{
    return (i < 0 || i >= (int)std::size(THEME_PRESETS)) ? "Custom" : THEME_PRESETS[i].name;
}

int Theme::theme_preset_index(std::string_view n)
{
    for (int i = 0; i < (int)std::size(THEME_PRESETS); ++i) if (n == THEME_PRESETS[i].name) return i;
    return -1;
}

void EditorSettings::load()
{
    std::ifstream f(Paths::roaming_data() / "editor_settings.cfg");
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        size_t sp = line.rfind(' ');
        if (sp == std::string::npos) continue;
        std::string key   = line.substr(0, sp);
        std::string value = line.substr(sp + 1);

        if (key == "interface.theme.preset") { theme.preset = Theme::theme_preset_index(value); continue; }
        if (key == "interface.theme.base") { color_from_hex(value, theme.base); continue; }
        if (key == "interface.theme.accent") { color_from_hex(value, theme.accent); continue; }
        if (key == "interface.theme.use_system_accent") { theme.use_system_accent = std::atoi(value.c_str()) != 0; continue; }
        
        if (key == "debugger.profiler.enabled") { profiler_enabled = std::atoi(value.c_str()) != 0; continue; }

        if (key.size() > 5 && key.compare(key.size() - 5, 5, ".open") == 0) panel_open[key.substr(0, key.size() - 5)] = std::atoi(value.c_str()) != 0;
    }
}

void EditorSettings::save() const
{
    std::ofstream f(Paths::roaming_data() / "editor_settings.cfg");
    if (!f) return;

    f << "interface.theme.preset " << Theme::theme_preset_name(theme.preset) << '\n';
    f << "interface.theme.base " << color_to_hex(theme.base) << '\n';
    f << "interface.theme.accent " << color_to_hex(theme.accent) << '\n';
    f << "interface.theme.use_system_accent " << (theme.use_system_accent ? 1 : 0) << '\n';

    f << "debugger.profiler.enabled " << (profiler_enabled ? 1 : 0) << '\n';

    for (const auto& [name, open] : panel_open) f << name << ".open " << (open ? 1 : 0) << '\n';
}


}