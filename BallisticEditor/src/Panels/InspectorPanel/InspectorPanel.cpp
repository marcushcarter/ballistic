#include "InspectorPanel.h"
#include <imgui.h>

void PrettyDragFloat3(const char* title, glm::vec3& v, float stride = 0.1f, float min = -100.0f, float max = 100.0f) {
	ImGui::Text(title);
	ImGui::SameLine(90);
	ImGui::PushID(title);
	ImGui::PushItemWidth(60);
	ImGui::DragFloat("X", &v.x, stride); ImGui::SameLine();
	ImGui::DragFloat("Y", &v.y, stride); ImGui::SameLine();
	ImGui::DragFloat("Z", &v.z, stride);
	ImGui::PopItemWidth();
	ImGui::PopID();
}

namespace Ballistic {

    InspectorPanel::InspectorPanel(std::shared_ptr<ProjectManager> projectManager) {
        m_ProjectManager = projectManager;
    }
	
	void InspectorPanel::init() {
	}

	void InspectorPanel::OnImGuiRender() {
        std::shared_ptr<Scene> currentScene = m_ProjectManager->GetSceneManager()->m_activeScene;
        auto& reg = currentScene->registry;

        ImGui::Begin("Inspector");

        if (currentScene->selected == entt::null) {
            ImGui::Text("No Entity Selected");
            ImGui::End();
            return;
        }

        bool hasTransform = reg.any_of<Transform>(currentScene->selected);
        bool hasSphere = reg.any_of<Sphere>(currentScene->selected);

        if (hasTransform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Transform& transform = reg.get<Transform>(currentScene->selected);

            ImGui::PushID("TransformSection");

            PrettyDragFloat3("Position", transform.position);
            PrettyDragFloat3("Rotation", transform.rotation);
            PrettyDragFloat3("Scale", transform.scale);

            ImGui::PopID();
            ImGui::Separator();
        }

        if (hasSphere && ImGui::CollapsingHeader("Sphere", ImGuiTreeNodeFlags_DefaultOpen)) {
            Sphere& sphere = reg.get<Sphere>(currentScene->selected);

            ImGui::PushID("SphereSection");

            ImGui::DragFloat("Radius", &sphere.radius, 0.1f, 0.0f, FLT_MAX);

            ImGui::PopID();
            ImGui::Separator();
        }

        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!hasTransform && ImGui::Selectable("Transform")) reg.emplace<Transform>(currentScene->selected);
            if (!hasSphere && ImGui::Selectable("Sphere")) reg.emplace<Sphere>(currentScene->selected);
            ImGui::EndPopup();
        }

        ImGui::End();
	}

    void InspectorPanel::onEvent(void* e) {
	}
}