#include "Panels/HierarchyPanel.h"

namespace ballistic
{
    HierarchyPanel::HierarchyPanel(LayerContext& context, const std::string& name) 
        : IPanel(context, name) {}
    
    void HierarchyPanel::OnAttach() {
    }

    void FlattenHierarchy(Scene* scene, entt::entity node, std::vector<entt::entity>& out) {
        auto& reg = scene->GetRegistry();
        out.push_back(node);
        if (reg.all_of<Children>(node)) {
            for (auto child : reg.get<Children>(node).children)
                FlattenHierarchy(scene, scene->GetEntity(child), out);
        }
    }

    bool ParentMatchesSearch(Scene* scene, entt::entity entity, const std::function<bool(const std::string&)>& matchesSearch) {
        auto& reg = scene->GetRegistry();
        if (!reg.all_of<Children>(entity)) return false;

        for (auto childID : reg.get<Children>(entity).children) {
            auto childEntity = scene->GetEntity(childID);
            auto& childTag = reg.get<Tag>(childEntity);
            if (matchesSearch(childTag.name)) return true;
            if (ParentMatchesSearch(scene, childEntity, matchesSearch)) return true;
        }
        return false;
    }
    
    void HierarchyPanel::OnDetach() {
    }

    void HierarchyPanel::OnUpdate(float deltaTime) {
        auto sceneManager = m_context.sceneManager;
        
        static ImGuiWindowFlags HierarchyFlags = ImGuiWindowFlags_NoCollapse;
		ImGui::Begin("Scene Hierarchy", nullptr, HierarchyFlags);

        if (sceneManager->HasActiveScene()) {
            
            auto scene = m_context.sceneManager->GetActiveScene();
            auto& reg = scene->GetRegistry();
            auto& selected = scene->GetSelected();
            
            static char searchBuffer[128] = "";

            ImGui::PushID("SearchBar");
            // float totalWidth = ImGui::GetContentRegionAvail().x;
            float buttonWidth = 25.0f;
            // float spacing = ImGui::GetStyle().ItemSpacing.x;
            float searchWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
            ImGui::PushItemWidth(searchWidth);
            ImGui::InputTextWithHint("##search", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer));
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button("?", ImVec2(buttonWidth, 0))) scene->Create("New Node", selected);
            ImGui::PopID();

            ImGui::BeginChild("HierarchyList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            std::string searchStr(searchBuffer);
            std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

            auto MatchesSearch = [&](const std::string& name) {
                if (searchStr.empty()) return true;
                std::string lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                return lowerName.find(searchStr) != std::string::npos;
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

        } else {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (ImGui::Button("New Scene", ImVec2(avail.x, 0))) {
                sceneManager->createScene("New Scene");
                auto selected = sceneManager->GetActiveScene()->Create("Root Node", sceneManager->GetActiveScene()->GetSelected());
                // sceneManager->GetActiveScene()->SetSelected(selected);
            }
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

        std::string nodeName = tag.name.empty() ? "Unnamed Node" : tag.name;
        
        if (tag.name.empty())
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        bool open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", nodeName.c_str());
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
                if (ImGui::MenuItem("Add Child Node")) scene->Create("New Node", entity);
                ImGui::Separator();
                ImGui::BeginDisabled(true);
                if (ImGui::MenuItem("Cut")) {}
                if (ImGui::MenuItem("Copy")) {}
                ImGui::EndDisabled();
                ImGui::Separator();
                if (ImGui::MenuItem("Rename")) {}
                if (ImGui::MenuItem("Duplicate")) hierarchyMetadata.deferredDuplicate = selected;
            }

            if (ImGui::MenuItem("Delete")) {
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
            for (auto child : reg.get<Children>(entity).children) DrawNode(scene, scene->GetEntity(child), matchesSearch);
        }

        if (open) ImGui::TreePop();
    }

    bool HierarchyPanel::IsSelected(entt::entity entity) const {
        return std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), entity) != hierarchyMetadata.multiSelection.end();
    }

    void HierarchyPanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
