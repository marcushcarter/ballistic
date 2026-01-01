#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class HierarchyPanel : public IPanel {
	public:
        HierarchyPanel(LayerContext& context, PanelStack& panelStack, const std::string& name = "Hierarchy");
        ~HierarchyPanel() override { OnDetach(); }
        
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnEvent(IEvent& e) override;

    private:        
        struct HierarchyMetaData {
            std::vector<entt::entity> multiSelection;
            entt::entity lastClicked = entt::null;
            entt::entity shiftAnchor = entt::null;

            std::vector<entt::entity> deferredDestroy;
            entt::entity deferredDuplicate = entt::null;
        };
        HierarchyMetaData hierarchyMetadata;
        
        GUID DrawSceneSelector(int& currentIndex);
        void DrawSceneInfo(Scene* scene);
        std::string DrawSearchBar();

        void DrawNode(Scene* scene, entt::entity e, const std::function<bool(const std::string&)>& matchesSearch);
        void FlattenHierarchy(Scene* scene, entt::entity node, std::vector<entt::entity>& out);
        bool ParentMatchesSearch(Scene* scene, entt::entity entity, const std::function<bool(const std::string&)>& matchesSearch);
        bool IsSelected(entt::entity e) const;
	};
}