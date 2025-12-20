#include "InspectorPanel.h"

namespace Ballistic {

    InspectorPanel::InspectorPanel(std::shared_ptr<ProjectManager> projectManager) {
        m_ProjectManager = projectManager;
    }
	
	void InspectorPanel::Init() {
	}

	void InspectorPanel::OnImGuiRender() {
        auto& scene = m_ProjectManager->GetSceneManager()->GetActiveScene();
        // auto& reg = scene.registry;
        EntityHandle selected(scene.GetSelected(), scene.GetRegistry());

        static ImGuiWindowFlags InspectorFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Inspector", nullptr, InspectorFlags);

        if (!selected.valid()) {
            ImGui::Text("No Entity Selected");
            ImGui::End();
            return;
        }

        bool hasTransform = selected.has<TransformComponent>();
        bool hasSphere = selected.has<SphereComponent>();
        bool hasMesh = selected.has<MeshComponent>();
        bool hasMaterial = selected.has<MaterialComponent>();
        bool hasCamera = selected.has<CameraComponent>();

		ImVec4 lightGray = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        if (selected.has<Tag>()) {
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

        if (selected.has<GUID>()) {
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
                ImGui::DragFloat3("Position", &transform.position.x);
                ImGui::DragFloat3("Rotation", &transform.rotation.x);
                ImGui::DragFloat3("Scale", &transform.scale.x);
            });
        }
        
        if (hasSphere) {
            DrawComponent<SphereComponent>("Sphere Component", selected, [&]() {
                auto& sph = selected.get<SphereComponent>();
                ImGui::DragFloat("Radius", &sph.radius, 0.1f, 0.0f, FLT_MAX);
            });
        }
        
        if (hasMesh) {
            DrawComponent<MeshComponent>("Mesh Component", selected, [&]() {
                auto& mesh = selected.get<MeshComponent>();
            });
        }
        
        if (hasMaterial) {
            DrawComponent<MaterialComponent>("Material Component", selected, [&]() {
                auto& mat = selected.get<MaterialComponent>();
            });
        }
        
        if (hasCamera) {
            DrawComponent<CameraComponent>("Camera Component", selected, [&]() {
                auto& cam = selected.get<CameraComponent>();
            });
        }

        bool hasAll = hasTransform && hasSphere && hasMesh && hasMaterial && hasCamera;

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (!hasAll) {
            if (ImGui::Button("Add Component", ImVec2(avail.x, 0)) && !hasAll) ImGui::OpenPopup("AddComponentPopup");
            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (!hasTransform && ImGui::Selectable("Transform")) selected.add<TransformComponent>();
                if (!hasSphere && ImGui::Selectable("Sphere")) selected.add<SphereComponent>();
                if (!hasMesh && ImGui::Selectable("Mesh")) selected.add<MeshComponent>();
                if (!hasMaterial && ImGui::Selectable("Material")) selected.add<MaterialComponent>();
                if (!hasCamera && ImGui::Selectable("Camera")) selected.add<CameraComponent>();
                ImGui::EndPopup();
            }
        }
        
        ImGui::End();
	}

    void InspectorPanel::OnEvent(void* e) {
	}
}