#pragma once
#include "Panels/IPanel.h"
#include "imgui.h"

namespace ballistic
{
	class HierarchyPanel : public IPanel {
	public:
        HierarchyPanel(LayerContext& context, const std::string& name = "Hierarchy");
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
        
        bool IsSelected(entt::entity e) const;
        void DrawNode(Scene* scene, entt::entity e, const std::function<bool(const std::string&)>& matchesSearch);
	};
}