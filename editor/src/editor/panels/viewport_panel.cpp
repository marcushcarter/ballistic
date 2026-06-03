#include "viewport_panel.h"
#include "editor/context.h"
#include "graphics/renderer.h"

void ViewportPanel::Draw(EditorContext& ctx)
{
    if (ImGui::Begin(ICON_FA_EYE " Viewport"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove) {
        
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x < 1.0f) size.x = 1.0f;
        if (size.y < 1.0f) size.y = 1.0f;
        uint32_t w = (uint32_t)size.x;
        uint32_t h = (uint32_t)size.y;

        if (w != pendingViewportW || h != pendingViewportH) {
            pendingViewportW = w;
            pendingViewportH = h;
            viewportSizeChanged = true;
        }

        if (viewportSizeChanged && !ImGui::IsAnyItemActive()) {
            if (w > 0 && h > 0 && (w != lastViewportW || h != lastViewportH)) {
                lastViewportW = w;
                lastViewportH = h;
                ctx.renderer.RequestSceneResize(w, h);
            }
            viewportSizeChanged = false;
        }

        ImGui::Image((ImTextureID)ctx.finalTextureID, size);

        float buttonPadding = 6.0f;
        ImVec2 buttonSize = ImVec2(24, 24);
        ImVec2 buttonStart = ImVec2(ImGui::GetWindowSize().x - buttonSize.x - ImGui::GetStyle().WindowPadding.x, ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y + buttonPadding);

        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y));
        if (ImGui::Button(ICON_FA_HAND_DOTS, buttonSize)) {}
    }
    ImGui::End();
}
