#include "Panels/ViewportPanel.h"

namespace ballistic
{
    ViewportPanel::ViewportPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void ViewportPanel::OnAttach() {

    }
    
    void ViewportPanel::OnDetach() {
    }

    void ViewportPanel::OnUpdate(float deltaTime) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        static ImGuiWindowFlags ViewportFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin((const char*)u8"\uF06E Viewport", nullptr, ViewportFlags);

        ImVec2 viewportWindowSize = ImGui::GetContentRegionAvail();
        double windowAspect = double(viewportWindowSize.x) / double(viewportWindowSize.y);

        if (windowAspect < ASPECT) {
            viewportSize.x = viewportWindowSize.x;
            viewportSize.y = viewportWindowSize.x / ASPECT;
        } else {
            viewportSize.x = viewportWindowSize.y * ASPECT;
            viewportSize.y = viewportWindowSize.y;
        }

        if (viewportSize.x != prevViewportSize.x || viewportSize.y != prevViewportSize.y) {
            glm::vec2 dim = glm::vec2(viewportSize.x, viewportSize.y);

            // m_renderer->RequestResize(dim);
            // camera.setScreenSize(dim);
            wasImguiInput = true;

            prevViewportSize = viewportSize;
        }

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 mouse = ImGui::GetMousePos();
        ImVec2 offset((viewportWindowSize.x - viewportSize.x) * 0.5f, (viewportWindowSize.y - viewportSize.y) * 0.5f);
        glm::vec2 mouseRelative(mouse.x - cursor.x - offset.x, mouse.y - cursor.y - offset.y);
        glm::vec2 mouseCentered(mouseRelative.x - viewportSize.x * 0.5f, mouseRelative.y - viewportSize.y * 0.5f);

        topLeftTextureCoords = ImVec2(cursor.x + offset.x, cursor.y + offset.y);
        bottomRightTextureCoords = ImVec2(topLeftTextureCoords.x + viewportSize.x, topLeftTextureCoords.y + viewportSize.y);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        // drawList->AddImage(
        //     m_renderer->GetDevice()->GetNativeTextureHandle(),
        //     topLeftTextureCoords, 
        //     bottomRightTextureCoords, 
        //     ImVec2(0,1), 
        //     ImVec2(1,0)
        // );
        drawList->AddRectFilled(
            topLeftTextureCoords,
            bottomRightTextureCoords,
            IM_COL32(255, 255, 255, 255)
        );
        
        // ImGuizmoGUI(scene, renderer, selected, topLeftTextureCoords, viewportSize, camera.m_View, camera.m_Projection);
        
        float buttonPadding = 6.0f;
        ImVec2 buttonSize = ImVec2(24, 24);
        ImVec2 buttonStart = ImVec2(
            ImGui::GetWindowSize().x - buttonSize.x - ImGui::GetStyle().WindowPadding.x,
            ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y + buttonPadding
        );

        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y));
        if (ImGui::Button((const char*)u8"\uF141##1", buttonSize)) {
            ImGui::OpenPopup("ViewportButtonMenu");
        }

        if (ImGui::BeginPopup("ViewportButtonMenu")) {
            if (ImGui::MenuItem("Option 1")) { /* handle click */ }
            if (ImGui::MenuItem("Option 2")) { /* handle click */ }
            if (ImGui::MenuItem("Option 3")) { /* handle click */ }

            ImGui::EndPopup();
        }
        
        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y + 1 * (buttonSize.y + buttonPadding)));
        if (ImGui::Button((const char*)u8"\uF142##2", buttonSize)) {}
        
        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y + 2 * (buttonSize.y + buttonPadding)));
        if (ImGui::Button((const char*)u8"\uF065##3", buttonSize)) {}

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
