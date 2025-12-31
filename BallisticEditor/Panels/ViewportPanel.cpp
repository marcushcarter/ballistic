#include "Panels/ViewportPanel.h"
#include <ImGuizmo.h>

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

        ImGuiWindowFlags ViewportFlags = ImGuiWindowFlags_NoCollapse;
        if (ImGuizmo::IsUsing() || ImGuizmo::IsOver())
            ViewportFlags |= ImGuiWindowFlags_NoMove;
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

            m_context.renderer->RequestResize(dim);
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
            m_context.renderer->GetDevice()->GetNativeTextureHandle(),
            topLeftTextureCoords, 
            bottomRightTextureCoords, 
            ImVec2(0,1), 
            ImVec2(1,0)
        );
        // drawList->AddRectFilled(
        //     topLeftTextureCoords,
        //     bottomRightTextureCoords,
        //     IM_COL32(255, 255, 255, 255)
        // );
        
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

        ImGui::PopStyleVar();

        if (ImGui::BeginPopup("ViewportButtonMenu")) {
            // static bool useMainCamera = false;
            if (ImGui::Checkbox("Use Main Camera", &m_context.renderer->useMainCamera)) {}
            // if (ImGui::MenuItem("Option 1")) { /* handle click */ }
            // if (ImGui::MenuItem("Option 2")) { /* handle click */ }
            // if (ImGui::MenuItem("Option 3")) { /* handle click */ }

            ImGui::EndPopup();
        }
        
        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y + 1 * (buttonSize.y + buttonPadding)));
        if (ImGui::Button((const char*)u8"\uF142##2", buttonSize)) {}
        
        ImGui::SetCursorPos(ImVec2(buttonStart.x, buttonStart.y + 2 * (buttonSize.y + buttonPadding)));
        if (ImGui::Button((const char*)u8"\uF065##3", buttonSize)) {}

        auto* scene = m_context.sceneManager->GetActiveScene();
        EntityHandle e(scene->GetSelected(), scene->GetRegistry());
        if (e.has<TransformComponent>()) {
            auto& t = e.get<TransformComponent>();
            
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
            ImGuizmo::SetRect(
                topLeftTextureCoords.x,
                topLeftTextureCoords.y,
                viewportSize.x,
                viewportSize.y
            );

            glm::mat4 view = m_context.renderer->GetDevice()->m_renderParams.camView;
            glm::mat4 proj = m_context.renderer->GetDevice()->m_renderParams.camProj;

            glm::mat4 worldMatrix = scene->ComputeWorldTransform(scene->GetSelected());

            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                ImGuizmo::TRANSLATE,
                ImGuizmo::WORLD,
                glm::value_ptr(worldMatrix)
            );

            if (ImGuizmo::IsUsing()) {

                glm::mat4 parentWorld(1.0f);
                entt::entity parent = entt::null;

                if (scene->GetRegistry().all_of<Parent>(scene->GetSelected()))
                    parent = scene->ConvertEntity(scene->GetRegistry().get<Parent>(scene->GetSelected()).parent);

                while (parent != entt::null && !scene->GetRegistry().all_of<TransformComponent>(parent)) {
                    if (!scene->GetRegistry().all_of<Parent>(parent)) {
                        parent = entt::null;
                        break;
                    }
                    parent = scene->ConvertEntity(scene->GetRegistry().get<Parent>(parent).parent);
                }

                if (parent != entt::null)
                    parentWorld = scene->ComputeWorldTransform(parent);

                glm::mat4 local = glm::inverse(parentWorld) * worldMatrix;

                glm::vec3 lTrans, lRot, lScale;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(local),
                    glm::value_ptr(lTrans),
                    glm::value_ptr(lRot),
                    glm::value_ptr(lScale)
                );

                t.position = lTrans;
                t.rotation = lRot;
                t.scale = lScale;

            }
        }

        ImGui::End();
    }

    void ViewportPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
