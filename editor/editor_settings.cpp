#include <editor/editor_settings.h>
#include <drivers/windows/window_driver_win32.h>
#include <cstdint>

namespace ballistic {

static ImVec4 shade(const ImVec4& c, float f) { return { c.x*f, c.y*f, c.z*f, c.w }; }
static ImVec4 fade(const ImVec4& c, float a) { return { c.x, c.y, c.z, a }; }
static ImVec4 mix(const ImVec4& a, const ImVec4& b, float t) { return { a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t, a.w }; }

void EditorSettings::Theme::apply() const
{
    ImVec4 acc = accent;
    if (use_system_accent) {
        float r, g, b;
        if (drivers::WindowDriverWin32::system_accent_color(r, g, b)) acc = ImVec4(r, g, b, 1.0f);
    }

    ImGui::StyleColorsDark();
    ImVec4* c = ImGui::GetStyle().Colors;

    c[ImGuiCol_WindowBg]             = base;
    c[ImGuiCol_ChildBg]              = shade(base, 0.92f);
    c[ImGuiCol_PopupBg]              = shade(base, 0.85f);
    c[ImGuiCol_MenuBarBg]            = shade(base, 0.90f);
    c[ImGuiCol_TitleBg]              = shade(base, 0.80f);
    c[ImGuiCol_TitleBgActive]        = shade(base, 0.90f);
    c[ImGuiCol_TitleBgCollapsed]     = shade(base, 0.70f);
    c[ImGuiCol_DockingEmptyBg]       = shade(base, 0.70f);
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.45f);

    // Frames (inputs, checkboxes, combos)
    c[ImGuiCol_FrameBg]              = mix(shade(base, 1.25f), acc, 0.06f);
    c[ImGuiCol_FrameBgHovered]       = mix(shade(base, 1.45f), acc, 0.18f);
    c[ImGuiCol_FrameBgActive]        = mix(shade(base, 1.60f), acc, 0.30f);

    // Lines
    c[ImGuiCol_Border]               = shade(base, 1.80f);
    c[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_Separator]            = shade(base, 1.80f);
    c[ImGuiCol_SeparatorHovered]     = fade(acc, 0.70f);
    c[ImGuiCol_SeparatorActive]      = acc;

    // Window resize grip
    c[ImGuiCol_ResizeGrip]           = fade(acc, 0.20f);
    c[ImGuiCol_ResizeGripHovered]    = fade(acc, 0.50f);
    c[ImGuiCol_ResizeGripActive]     = acc;

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]          = shade(base, 0.80f);
    c[ImGuiCol_ScrollbarGrab]        = shade(base, 1.60f);
    c[ImGuiCol_ScrollbarGrabHovered] = fade(acc, 0.50f);
    c[ImGuiCol_ScrollbarGrabActive]  = acc;

    // Interactive
    c[ImGuiCol_Header]               = fade(acc, 0.35f);
    c[ImGuiCol_HeaderHovered]        = fade(acc, 0.55f);
    c[ImGuiCol_HeaderActive]         = acc;
    c[ImGuiCol_Button]               = fade(acc, 0.30f);
    c[ImGuiCol_ButtonHovered]        = fade(acc, 0.50f);
    c[ImGuiCol_ButtonActive]         = acc;
    c[ImGuiCol_CheckMark]            = acc;
    c[ImGuiCol_SliderGrab]           = acc;
    c[ImGuiCol_SliderGrabActive]     = acc;

    // Tabs
    c[ImGuiCol_Tab]                  = shade(base, 0.90f);
    c[ImGuiCol_TabHovered]           = fade(acc, 0.60f);
    c[ImGuiCol_TabActive]            = fade(acc, 0.80f);
    c[ImGuiCol_TabUnfocused]         = shade(base, 0.85f);
    c[ImGuiCol_TabUnfocusedActive]   = shade(base, 1.05f);

    // Docking / drag-drop / nav
    c[ImGuiCol_DockingPreview]       = fade(acc, 0.60f);
    c[ImGuiCol_DragDropTarget]       = acc;
    c[ImGuiCol_NavHighlight]         = acc;
    c[ImGuiCol_TextSelectedBg]       = fade(acc, 0.35f);

    // Tables
    c[ImGuiCol_TableHeaderBg]        = shade(base, 1.20f);
    c[ImGuiCol_TableBorderStrong]    = shade(base, 1.90f);
    c[ImGuiCol_TableBorderLight]     = shade(base, 1.50f);
    c[ImGuiCol_TableRowBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt]        = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
}

}