#include "Panels/InspectorPanel.h"

namespace ballistic
{
    InspectorPanel::InspectorPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void InspectorPanel::OnAttach() {
    }
    
    void InspectorPanel::OnDetach() {
    }

    void InspectorPanel::OnUpdate(float deltaTime) {
        auto sceneManager = m_context.sceneManager;

        static ImGuiWindowFlags InspectorFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin((const char*)u8"\uF1DE Inspector", nullptr, InspectorFlags);

        ImVec4 lightGray = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        if (sceneManager->HasActiveScene()) {
            auto scene = m_context.sceneManager->GetActiveScene();
            auto& reg = scene->GetRegistry();
            EntityHandle selected(scene->GetSelected(), scene->GetRegistry());

            if (!selected.valid()) {
                ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
                ImGui::Text("No Entity Selected");
                ImGui::PopStyleColor();
                ImGui::End();
                return;
            }

            bool hasParent = selected.has<Parent>();
            bool hasChildren = selected.has<Children>();

            bool hasTransform = selected.has<TransformComponent>();
            bool hasSphere = selected.has<SphereComponent>();
            bool hasMesh = selected.has<MeshComponent>();
            bool hasMaterial = selected.has<MaterialComponent>();
            bool hasCamera = selected.has<CameraComponent>();

            if (selected.has<Tag>()) {
                auto& tag = selected.get<Tag>();

                char buffer[256];
                strncpy(buffer, tag.name.c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0';
                
                ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
                ImGui::Text((const char*)u8"\uF02C Name:");
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                
                if (ImGui::InputTextWithHint("##Name", "Unnamed Node", buffer, IM_ARRAYSIZE(buffer))) {
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
                DrawComponent<TransformComponent>((const char*)u8"\uF047 Transform Component", selected, [&]() {
                    auto& transform = selected.get<TransformComponent>();
                    ImGui::DragFloat3("Position", &transform.position.x);
                    ImGui::DragFloat3("Rotation", &transform.rotation.x);
                    ImGui::DragFloat3("Scale", &transform.scale.x);
                });
            }
            
            if (hasSphere) {
                DrawComponent<SphereComponent>((const char*)u8"\u25CF Sphere Component", selected, [&]() {
                    auto& sph = selected.get<SphereComponent>();
                    ImGui::DragFloat("Radius", &sph.radius, 0.1f, 0.0f, FLT_MAX);
                });
            }
            
            if (hasMesh) {
                DrawComponent<MeshComponent>((const char*)u8"\uF1C0 Mesh Component", selected, [&]() {
                    auto& mesh = selected.get<MeshComponent>();

                    auto meshManager = GetRoot()->GetMeshManager();

                    const auto& allMetadata = meshManager->GetAllMetadata();
                    if (!allMetadata.empty()) {
                        std::vector<std::string> names;
                        int currentIndex = 0;
                        for (size_t i = 0; i < allMetadata.size(); ++i) {
                            names.push_back(allMetadata[i].name);
                            if (allMetadata[i].guid == mesh.mesh)
                                currentIndex = static_cast<int>(i);
                        }

                        // Dropdown
                        if (ImGui::BeginCombo("Mesh", names[currentIndex].c_str())) {
                            for (int i = 0; i < names.size(); ++i) {
                                bool isSelected = (i == currentIndex);
                                if (ImGui::Selectable(names[i].c_str(), isSelected)) {
                                    currentIndex = i;
                                    mesh.mesh = allMetadata[i].guid;
                                }
                                if (isSelected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    } else {
                        ImGui::TextDisabled("No meshes available");
                    }
                });
            }
            
            if (hasMaterial) {
                DrawComponent<MaterialComponent>((const char*)u8"\uF043 Material Component", selected, [&]() {
                    auto& mat = selected.get<MaterialComponent>();
                });
            }
            
            if (hasCamera) {
                DrawComponent<CameraComponent>((const char*)u8"\uF03D Camera Component", selected, [&]() {
                    auto& cam = selected.get<CameraComponent>();
                    bool isMain = cam.mainCamera;
                    if (ImGui::Checkbox("Main Camera", &isMain)) scene->SetMainCamera(selected.handle());
                    if (ImGui::DragFloat("FOV", &cam.fov, 0.1f, 1.0f, 179.0f)) {}
                });
            }

            // if (hasParent || hasChildren) {
            //     DrawComponent<void>("Heirarchy", selected, [&]() {
            //         if (hasParent) {
            //             auto& parent = selected.get<Parent>();
            //             ImGui::SeparatorText("Parent");
            //             ImGui::PushID("Parent");
            //             EntityHandle p(scene->GetEntity(parent.parent), reg);
            //             if (ImGui::Button(p.get<Tag>().name.c_str())) {
            //                     scene->SetSelected(p.handle());
            //             }
            //             ImGui::PopID();
            //         }

            //         if (hasChildren) {
            //             auto& children = selected.get<Children>();
            //             ImGui::SeparatorText("Children");
            //             for (auto& child : children.children) {
            //                 EntityHandle c(scene->GetEntity(child), reg);
            //                 ImGui::PushID("c.handle()");
            //                 if (ImGui::Button(c.get<Tag>().name.c_str())) {
            //                     scene->SetSelected(c.handle());
            //                 }
            //                 ImGui::PopID();
            //             }
            //         }
            //     });
            // }

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
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
            ImGui::Text("No Scene Selected");
            ImGui::PopStyleColor();
        }
        
        ImGui::End();
    }

    void InspectorPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
