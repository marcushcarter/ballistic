#include "Panels/HierarchyPanel.h"

namespace ballistic
{
    HierarchyPanel::HierarchyPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void HierarchyPanel::OnAttach() {
    }
    
    void HierarchyPanel::OnDetach() {
    }

    GUID HierarchyPanel::DrawSceneSelector(int& currentIndex) {
        auto sceneManager = m_context.sceneManager;
        std::vector<GUID> sceneGUIDs;

        for (auto& scenePtr : sceneManager->GetAllScenes())
            sceneGUIDs.push_back(sceneManager->ConvertGUID(scenePtr.get()));

        sceneGUIDs.push_back(GUID::Invalid);

        float availWidth = ImGui::GetContentRegionAvail().x;

        if (sceneManager->GetAllScenes().empty()) {
            if (ImGui::Button("Create New Scene", ImVec2(availWidth, 0))) {
                auto newScene = sceneManager->Create("New Scene");
                sceneManager->SetActiveScene(sceneManager->ConvertGUID(newScene.get()));
                currentIndex = 0;
            }
            return sceneManager->HasActiveScene() ? sceneManager->GetActiveScene()->GetGUID() : GUID::Invalid;
        } else {
            const char* preview = "Select Scene";
            if (currentIndex < sceneGUIDs.size() - 1) {
                Scene* selectedScene = sceneManager->ConvertScene(sceneGUIDs[currentIndex]);
                if (selectedScene) preview = selectedScene->GetName().empty() ? "Unnamed Scene" : selectedScene->GetName().c_str();
            } else {
                preview = "Create New Scene";
            }

            ImGui::PushItemWidth(availWidth);
            if (ImGui::BeginCombo("##Scenes", preview)) {
                for (int i = 0; i < sceneGUIDs.size(); i++) {
                    const bool isSelected = (currentIndex == i);
                    const char* name;

                    if (i < sceneGUIDs.size() - 1) {
                        Scene* scene = sceneManager->ConvertScene(sceneGUIDs[i]);
                        name = (scene && !scene->GetName().empty()) ? scene->GetName().c_str() : "Unnamed Scene";
                    } else {
                        name = "Create New Scene";
                    }

                    ImGui::PushID(static_cast<int>(sceneGUIDs[i].value));
                    if (ImGui::Selectable(name, isSelected)) {
                        if (i == sceneGUIDs.size() - 1) {
                            auto newScene = sceneManager->Create("New Scene");
                            sceneManager->SetActiveScene(sceneManager->ConvertGUID(newScene.get()));
                            currentIndex = static_cast<int>(sceneManager->GetAllScenes().size()) - 1;
                        } else {
                            currentIndex = i;
                            sceneManager->SetActiveScene(sceneGUIDs[i]);
                        }
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            return sceneManager->GetActiveScene()->GetGUID();
        }
    }

    void HierarchyPanel::DrawSceneInfo(Scene* scene) {
        ImVec4 lightGray = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        char buffer[256];
        strncpy(buffer, scene->GetName().c_str(), sizeof(buffer));
        buffer[sizeof(buffer)-1] = '\0';

        ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
        ImGui::Text((const char*)u8"\uF02C Name:");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextWithHint("##Name", "Unnamed Scene", buffer, IM_ARRAYSIZE(buffer))) {
            scene->SetName(std::string(buffer));
        }

        ImGui::PushStyleColor(ImGuiCol_Text, lightGray);
        ImGui::Text("Guid:");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Text("%llu", scene->GetGUID().value);

        ImGui::BeginDisabled(true);
        if (ImGui::Button("Delete")) {
            m_context.sceneManager->Destroy(scene->GetGUID());
        }
        ImGui::EndDisabled();
    }

    std::string HierarchyPanel::DrawSearchBar() {
        static char searchBuffer[128] = "";
        ImGui::PushID("SearchBar");

        float buttonWidth = 25.0f;
        float searchWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
        ImGui::PushItemWidth(searchWidth);

        ImGui::InputTextWithHint("##search", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button((const char*)u8"\uF0FE", ImVec2(buttonWidth, 0))) {
            auto scene = m_context.sceneManager->GetActiveScene();
            if (scene) scene->Create("New Node", scene->GetSelected());
        }

        ImGui::PopID();
        return std::string(searchBuffer);
    }

    void HierarchyPanel::OnUpdate(float deltaTime) {
        auto sceneManager = m_context.sceneManager;
        
        static ImGuiWindowFlags HierarchyFlags = ImGuiWindowFlags_NoCollapse;
		ImGui::Begin((const char*)u8"\uF080 Scene Hierarchy", nullptr, HierarchyFlags);

        static int currentIndex = 0;
        GUID activeGUID = DrawSceneSelector(currentIndex);

        if (sceneManager->HasActiveScene()) {
            
            auto scene = m_context.sceneManager->GetActiveScene();
            auto& reg = scene->GetRegistry();
            auto& selected = scene->GetSelected();

            DrawSceneInfo(scene);
            std::string searchStr = DrawSearchBar();

            ImGui::BeginChild("HierarchyList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            auto MatchesSearch = [&](const std::string& name) {
                std::string lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                std::string searchLower = searchStr;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                return searchLower.empty() || lowerName.find(searchLower) != std::string::npos;
            };

            for (auto e : reg.view<Tag>()) {
                if (!reg.all_of<Parent>(e))
                    DrawNode(scene, e, MatchesSearch);
            }

            for (auto e : hierarchyMetadata.deferredDestroy) scene->Destroy(e);
            hierarchyMetadata.deferredDestroy.clear();

            if (hierarchyMetadata.deferredDuplicate != entt::null) {
                scene->Duplicate(hierarchyMetadata.deferredDuplicate);
                hierarchyMetadata.deferredDuplicate = entt::null;
            }

            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY());
            if (ImGui::InvisibleButton("##root_drop_target", avail) || (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())) {
                hierarchyMetadata.multiSelection.clear();
                selected = entt::null;
                hierarchyMetadata.shiftAnchor = entt::null;
                hierarchyMetadata.lastClicked = entt::null;
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD_MULTI")) {
                    size_t count = payload->DataSize / sizeof(entt::entity);
                    entt::entity* dragged = (entt::entity*)payload->Data;
                    for (size_t i = 0; i < count; i++) scene->Reparent(dragged[i]);
                } else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD")) {
                    entt::entity dragged = *(entt::entity*)payload->Data;
                    scene->Reparent(dragged);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void HierarchyPanel::DrawNode(Scene* scene, entt::entity entity, const std::function<bool(const std::string&)>& matchesSearch) {
        auto& reg = scene->GetRegistry();
        auto& selected = scene->GetSelected();
        auto& tag = reg.get<Tag>(entity);

        bool hasChildren = reg.all_of<Children>(entity) && !reg.get<Children>(entity).children.empty();
        bool isSelected = selected == entity || std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), entity) != hierarchyMetadata.multiSelection.end();

        if (std::find(hierarchyMetadata.deferredDestroy.begin(), hierarchyMetadata.deferredDestroy.end(), entity) != hierarchyMetadata.deferredDestroy.end())
            return;

        if (!matchesSearch(tag.name) && !ParentMatchesSearch(scene, entity, matchesSearch))
            return;

        ImGuiTreeNodeFlags flags = 
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding;
            
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        const char* iconString = hasChildren
            ? reinterpret_cast<const char*>(u8"\uF1B3")
            : reinterpret_cast<const char*>(u8"\uF1B2");

        const char* nodeName = tag.name.empty()
            ? "Unnamed Node"
            : tag.name.c_str();

        const char* extraIcons = reinterpret_cast<const char*>(u8"\uF06E \uF070 \uF1F8");
        
        if (tag.name.empty())
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        
            bool open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s %s %s", iconString, nodeName, extraIcons);
        
        if (tag.name.empty())
            ImGui::PopStyleColor();

        if (ImGui::IsItemClicked()) {
            if (ImGui::GetIO().KeyShift && hierarchyMetadata.shiftAnchor != entt::null) {
                hierarchyMetadata.multiSelection.clear();
                std::vector<entt::entity> flatOrder;
                for (auto root : reg.view<Tag>()) if (!reg.all_of<Parent>(root)) FlattenHierarchy(scene, root, flatOrder);
                bool inRange = false;
                for (auto node : flatOrder) {
                    if (node == hierarchyMetadata.shiftAnchor || node == entity) {
                        if (!inRange) inRange = true;
                        else { hierarchyMetadata.multiSelection.push_back(node); break; }
                    }
                    if (inRange) hierarchyMetadata.multiSelection.push_back(node);
                }

            } else if (ImGui::GetIO().KeyCtrl) {
                auto it = std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), entity);
                if (it != hierarchyMetadata.multiSelection.end()) hierarchyMetadata.multiSelection.erase(it);
                else hierarchyMetadata.multiSelection.push_back(entity);

            } else {
                if (std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), entity) == hierarchyMetadata.multiSelection.end()) {
                    hierarchyMetadata.multiSelection = { entity };
                    hierarchyMetadata.shiftAnchor = entity;
                }
            }

            selected = (hierarchyMetadata.multiSelection.size() == 1) ? hierarchyMetadata.multiSelection.front() : entt::null;
            hierarchyMetadata.lastClicked = entity;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (hierarchyMetadata.multiSelection.size() <= 1) {
                if (ImGui::MenuItem((const char*)u8"\uF0FE Add Child Node")) scene->Create("New Node", entity);
                ImGui::Separator();
                ImGui::BeginDisabled(true);
                if (ImGui::MenuItem((const char*)u8"\u2700 Cut")) {}
                if (ImGui::MenuItem((const char*)u8"\uF0C5 Copy")) {}
                ImGui::EndDisabled();
                ImGui::Separator();
                if (ImGui::MenuItem((const char*)u8"\uF246 Rename")) {}
                if (ImGui::MenuItem((const char*)u8"\uF24D Duplicate")) hierarchyMetadata.deferredDuplicate = selected;
            }

            if (ImGui::MenuItem((const char*)u8"\uF1F8 Delete")) {
                hierarchyMetadata.deferredDestroy.insert(hierarchyMetadata.deferredDestroy.end(), hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end());
                if (std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), selected) != hierarchyMetadata.multiSelection.end()) 
                    selected = entt::null;
                hierarchyMetadata.multiSelection.clear();
            }

            ImGui::EndPopup();
        }
        
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            if (hierarchyMetadata.multiSelection.size() > 1) {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD_MULTI", hierarchyMetadata.multiSelection.data(), hierarchyMetadata.multiSelection.size() * sizeof(entt::entity));
                ImGui::Text("%d Entities", (int)hierarchyMetadata.multiSelection.size());
            } else {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD", &entity, sizeof(entt::entity));
                ImGui::Text("%s", tag.name.c_str());
            }
            ImGui::EndDragDropSource();
        }

        auto CanAcceptDrop = [&](entt::entity target) {
            if (hierarchyMetadata.multiSelection.size() > 1 && IsSelected(target)) return false;
            for (entt::entity dragged : hierarchyMetadata.multiSelection)
                if (scene->IsDescendant(dragged, target)) return false;
            return true;
        };

        if (CanAcceptDrop(entity) && ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD_MULTI")) {
                size_t count = payload->DataSize / sizeof(entt::entity);
                entt::entity* dragged = (entt::entity*)payload->Data;
                for (size_t i = 0; i < count; i++) if (dragged[i] != entity) scene->Reparent(dragged[i], entity);
            } else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD")) {
                entt::entity dragged = *(entt::entity*)payload->Data;
                if (dragged != entity) scene->Reparent(dragged, entity);
            }
            ImGui::EndDragDropTarget();
        }

        if (open && hasChildren) {
            for (auto child : reg.get<Children>(entity).children) DrawNode(scene, scene->ConvertEntity(child), matchesSearch);
        }

        if (open) ImGui::TreePop();
    }

    void HierarchyPanel::FlattenHierarchy(Scene* scene, entt::entity node, std::vector<entt::entity>& out) {
        auto& reg = scene->GetRegistry();
        out.push_back(node);
        if (reg.all_of<Children>(node)) {
            for (auto child : reg.get<Children>(node).children)
                FlattenHierarchy(scene, scene->ConvertEntity(child), out);
        }
    }

    bool HierarchyPanel::ParentMatchesSearch(Scene* scene, entt::entity entity, const std::function<bool(const std::string&)>& matchesSearch) {
        auto& reg = scene->GetRegistry();
        if (!reg.all_of<Children>(entity)) return false;

        for (auto childID : reg.get<Children>(entity).children) {
            auto childEntity = scene->ConvertEntity(childID);
            auto& childTag = reg.get<Tag>(childEntity);
            if (matchesSearch(childTag.name)) return true;
            if (ParentMatchesSearch(scene, childEntity, matchesSearch)) return true;
        }
        return false;
    }

    bool HierarchyPanel::IsSelected(entt::entity entity) const {
        return std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), entity) != hierarchyMetadata.multiSelection.end();
    }

    void HierarchyPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
