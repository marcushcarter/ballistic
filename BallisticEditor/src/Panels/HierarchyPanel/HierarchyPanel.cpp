#include "HierarchyPanel.h"

#include <imgui.h>

namespace Ballistic {

    HierarchyPanel::HierarchyPanel(std::shared_ptr<ProjectManager> projectManager) {
        m_ProjectManager = projectManager;
    }
	
	void HierarchyPanel::init() {
	}

    bool HierarchyPanel::IsDescendant(std::shared_ptr<Scene> scene, entt::entity node, entt::entity potentialChild) {
        if (!scene->registry.all_of<Children>(node)) return false;
        for (auto child : scene->registry.get<Children>(node).entities) {
            if (child == potentialChild) return true;
            if (IsDescendant(scene, child, potentialChild)) return true;
        }
        return false;
    }

    void HierarchyPanel::FlattenHierarchy(std::shared_ptr<Scene> scene, entt::entity node, std::vector<entt::entity>& out) {
        out.push_back(node);
        if (scene->registry.all_of<Children>(node)) {
            for (auto child : scene->registry.get<Children>(node).entities)
                FlattenHierarchy(scene, child, out);
        }
    }

    void HierarchyPanel::HandleNodeClick(std::shared_ptr<Scene> scene, entt::entity e) {
        if (!ImGui::IsItemClicked()) return;

        if (ImGui::GetIO().KeyShift && hierarchyMetadata.shiftAnchor != entt::null) {
            hierarchyMetadata.multiSelection.clear();
            std::vector<entt::entity> flatOrder;

            for (auto root : scene->registry.view<Tag>()) {
                if (!scene->registry.all_of<Parent>(root))
                    FlattenHierarchy(scene, root, flatOrder);
            }
            bool inRange = false;
            for (auto node : flatOrder) {
                if (node == hierarchyMetadata.shiftAnchor || node == e) {
                    if (!inRange) inRange = true;
                    else { hierarchyMetadata.multiSelection.push_back(node); break; }
                }
                if (inRange) hierarchyMetadata.multiSelection.push_back(node);
            }

        } else if (ImGui::GetIO().KeyCtrl) {
            
            auto it = std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), e);
            if (it != hierarchyMetadata.multiSelection.end()) hierarchyMetadata.multiSelection.erase(it);
            else hierarchyMetadata.multiSelection.push_back(e);

        } else {

            if (std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), e) == hierarchyMetadata.multiSelection.end()) {
                hierarchyMetadata.multiSelection = { e };
                hierarchyMetadata.shiftAnchor = e;
            }
        }

        scene->selected = (hierarchyMetadata.multiSelection.size() == 1) ? hierarchyMetadata.multiSelection.front() : entt::null;
        hierarchyMetadata.lastClicked = e;
    }

    void HierarchyPanel::DrawNode(std::shared_ptr<Scene> scene, entt::entity e) {
        auto& reg = scene->registry;
        auto& tag = reg.get<Tag>(e);
        bool hasChildren = reg.all_of<Children>(e) && !reg.get<Children>(e).entities.empty();
        bool isSelected = (scene->selected == e) || (std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), e) != hierarchyMetadata.multiSelection.end());

        if (std::find(hierarchyMetadata.deferredDestroy.begin(), hierarchyMetadata.deferredDestroy.end(), e) != hierarchyMetadata.deferredDestroy.end()) return;

        ImGuiTreeNodeFlags flags = 
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAllColumns |
            ImGuiTreeNodeFlags_FramePadding;
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::TreeNodeEx((void*)(intptr_t)e, flags, tag.name.c_str());

        HandleNodeClick(scene, e);

        if (ImGui::BeginPopupContextItem()) {

            if (hierarchyMetadata.multiSelection.size() > 1) {

            } else {

                if (ImGui::MenuItem("Add Child Node")) {
                    scene->create("New Node", e);
                }

                ImGui::Separator();
                ImGui::BeginDisabled(true);

                if (ImGui::MenuItem("Cut")) {
                }

                if (ImGui::MenuItem("Copy")) {
                }
                
                ImGui::EndDisabled();
                ImGui::Separator();

                if (ImGui::MenuItem("Rename")) {}
                
                if (ImGui::MenuItem("Duplicate")) hierarchyMetadata.deferredDuplicate = scene->selected;

            }

            if (ImGui::MenuItem("Delete")) {
                hierarchyMetadata.deferredDestroy.insert(hierarchyMetadata.deferredDestroy.end(), hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end());
                if (std::find(hierarchyMetadata.multiSelection.begin(), hierarchyMetadata.multiSelection.end(), scene->selected) != hierarchyMetadata.multiSelection.end()) 
                    scene->selected = entt::null;

                hierarchyMetadata.multiSelection.clear();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            if (hierarchyMetadata.multiSelection.size() > 1) {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD_MULTI", hierarchyMetadata.multiSelection.data(), hierarchyMetadata.multiSelection.size() * sizeof(entt::entity));
                ImGui::Text("%d Entities", (int)hierarchyMetadata.multiSelection.size());
            } else {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD", &e, sizeof(entt::entity));
                ImGui::Text("%s", tag.name.c_str());
            }
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD_MULTI")) {
                size_t count = payload->DataSize / sizeof(entt::entity);
                entt::entity* draggedEntities = (entt::entity*)payload->Data;
                for (size_t i = 0; i < count; i++) {
                    if (draggedEntities[i] != e && !IsDescendant(scene, draggedEntities[i], e)) {
                        scene->reparent(draggedEntities[i], e);
                    }
                }
            } else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD")) {
                entt::entity draggedEntity = *(entt::entity*)payload->Data;
                if (draggedEntity != e && !IsDescendant(scene, draggedEntity, e)) {
                    scene->reparent(draggedEntity, e);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (open && hasChildren) {
            auto& children = reg.get<Children>(e).entities;
            for (auto child : children) {
                DrawNode(scene, child);
            }
        }

        if (open) ImGui::TreePop();

    }

	void HierarchyPanel::OnImGuiRender() {
        std::shared_ptr<Scene> currentScene = m_ProjectManager->GetSceneManager()->m_activeScene;
        
		ImGui::Begin("Scene Hierarchy");

        if (ImGui::Button("create")) currentScene->create("New Anchor", currentScene->selected);
        ImGui::BeginDisabled(true);
        ImGui::SameLine();
        if (ImGui::Button("sort")) {} // sortChildren(scene, entt::null);
        ImGui::EndDisabled();
        // ImGui::SameLine();
        // if (ImGui::Button("exp.")) expandAll = true;
        // ImGui::SameLine();
        // if (ImGui::Button("col.")) collapseAll = true;

        ImGui::BeginChild("HierarchyList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (auto e : currentScene->registry.view<Tag>()) {
            if (!currentScene->registry.all_of<Parent>(e)) {
                DrawNode(currentScene, e);
            }
        }

        for (auto entity : hierarchyMetadata.deferredDestroy) currentScene->destroy(entity);
        hierarchyMetadata.deferredDestroy.clear();

        if (hierarchyMetadata.deferredDuplicate != entt::null) {
            currentScene->duplicate(hierarchyMetadata.deferredDuplicate);
            hierarchyMetadata.deferredDuplicate = entt::null;
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY());
        if (ImGui::InvisibleButton("##root_drop_target", avail)) {
            hierarchyMetadata.multiSelection.clear();
            currentScene->selected = entt::null;
            hierarchyMetadata.shiftAnchor = entt::null;
            hierarchyMetadata.lastClicked = entt::null;
        }

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
            hierarchyMetadata.multiSelection.clear();
            currentScene->selected = entt::null;
            hierarchyMetadata.shiftAnchor = entt::null;
            hierarchyMetadata.lastClicked = entt::null;
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD_MULTI")) {

                size_t count = payload->DataSize / sizeof(entt::entity);
                entt::entity* draggedEntities = (entt::entity*)payload->Data;
                for (size_t i = 0; i < count; i++) {
                    currentScene->reparent(draggedEntities[i]);
                }

            } else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_PAYLOAD")) {
                
                entt::entity draggedEntity = *(entt::entity*)payload->Data;
                currentScene->reparent(draggedEntity);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::EndChild();
        ImGui::End();
	}
	
	void HierarchyPanel::onEvent(void* e) {
	}
}