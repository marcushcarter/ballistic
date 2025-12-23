#include "ViewportPanel.h"

namespace Ballistic {

    ViewportPanel::ViewportPanel(std::shared_ptr<Renderer> renderer) {
        m_renderer = renderer;
    }
	
	void ViewportPanel::Init() {
	}

	void ViewportPanel::OnImGuiRender() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        static ImGuiWindowFlags ViewportFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Viewport", nullptr, ViewportFlags);

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

            m_renderer->RequestResize(dim);
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
        drawList->AddImage(
            m_renderer->GetDevice()->GetNativeTextureHandle(),
            topLeftTextureCoords, 
            bottomRightTextureCoords, 
            ImVec2(0,1), 
            ImVec2(1,0)
        );

        ImVec2 windowPos      = ImGui::GetWindowPos();
        ImVec2 windowSize     = ImGui::GetWindowSize();
        ImVec2 padding        = ImGui::GetStyle().WindowPadding;
        float titleBarHeight  = ImGui::GetFrameHeight(); // this is the key

        ImVec2 buttonSize = ImVec2(24, 24);

        // Cursor position is LOCAL to window
        ImGui::SetCursorPos(ImVec2(
            windowSize.x - buttonSize.x - padding.x,
            titleBarHeight + padding.y
        ));

        if (ImGui::Button("...", buttonSize)) {
            // clicked
        }

        // ImGuizmoGUI(scene, renderer, selected, topLeftTextureCoords, viewportSize, camera.m_View, camera.m_Projection);

        ImGui::End();
        ImGui::PopStyleVar();
	}
	
	void ViewportPanel::OnEvent(void* e) {
	}
}