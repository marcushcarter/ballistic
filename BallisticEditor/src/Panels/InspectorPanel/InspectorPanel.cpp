#include "InspectorPanel.h"

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
        auto& scene = *m_ProjectManager->GetSceneManager()->m_activeScene;
        // auto& reg = scene.registry;
        EntityHandle selected(scene.selected, scene.registry);

        static ImGuiWindowFlags InspectorFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Inspector", nullptr, InspectorFlags);

        if (!selected.valid()) {
            ImGui::Text("No Entity Selected");
            ImGui::End();
            return;
        }

        bool hasTag = selected.has<Tag>();
        bool hasGUID = selected.has<GUID>();
        bool hasTransform = selected.has<TransformComponent>();
        bool hasSphere = selected.has<SphereComponent>();

		ImVec4 lightGray = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        if (hasTag) {
            auto& tag = selected.get<Tag>();

		    static char buffer[256];
		    static entt::entity lastEntity = entt::null;

            if (selected.handle() != lastEntity) {
                strncpy(buffer, tag.name.c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0';
                lastEntity = selected.handle();
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
            ImGui::Text(ICON_FA_TAG " Name:");
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
		
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetNextItemWidth(avail.x);
            if  (ImGui::InputText("##Name", buffer, IM_ARRAYSIZE(buffer))) {
                tag.name = buffer;
            }
        }

        if (hasGUID) {
            auto& guid = selected.get<GUID>();
            
            ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
            ImGui::Text("Guid:");
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::Text("%llu", guid.value);
        }

        if (hasTransform) {
            DrawComponent<TransformComponent>("Transform Component", selected, [&]() {
                auto& transform = selected.get<TransformComponent>();
                PrettyDragFloat3("Position", transform.position);
                PrettyDragFloat3("Rotation", transform.rotation);
                PrettyDragFloat3("Scale", transform.scale);
            });
        }
        
        if (hasSphere) {
            DrawComponent<SphereComponent>("Sphere Component", selected, [&]() {
                auto& sphere = selected.get<SphereComponent>();
                ImGui::DragFloat("Radius", &sphere.radius, 0.1f, 0.0f, FLT_MAX);
            });
        }

        if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");
        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!hasTransform && ImGui::Selectable("Transform")) selected.add<TransformComponent>();
            if (!hasSphere && ImGui::Selectable("Sphere")) selected.add<SphereComponent>();
            ImGui::EndPopup();
        }

        ImGui::End();
	}

    void InspectorPanel::onEvent(void* e) {
	}
}