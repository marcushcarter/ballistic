#include <editor/settings/settings.h>
#include <editor/editor_settings.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

void Settings::draw_contents(EditorContext& ctx)
{    
    EditorSettings::Theme& t = ctx.settings->theme;
    bool changed = false;

    if (ImGui::BeginCombo("Preset", theme_preset_name(t.preset))) {
        for (int i = 0; i < (int)std::size(THEME_PRESETS); ++i) {
            if (ImGui::Selectable(THEME_PRESETS[i].name, t.preset == i)) {
                t.preset  = i;
                t.base    = THEME_PRESETS[i].base;
                t.accent  = THEME_PRESETS[i].accent;
                changed = true;
            }
        }
        if (ImGui::Selectable("Custom", t.preset == -1)) { t.preset = -1; changed = true; }
        ImGui::EndCombo();
    }

    if (ImGui::ColorEdit3("Base", &t.base.x)) { t.preset = -1; changed = true; }
    ImGui::BeginDisabled(t.use_system_accent);
    if (ImGui::ColorEdit3("Accent", &t.accent.x)) { t.preset = -1; changed = true; }
    ImGui::EndDisabled();
    if (ImGui::Checkbox("Use system accent", &t.use_system_accent)) changed = true;
    if (ImGui::Button("Reset to defaults")) {
        t = EditorSettings::Theme{};
        changed = true;
    }

    if (changed) t.apply();
}

}