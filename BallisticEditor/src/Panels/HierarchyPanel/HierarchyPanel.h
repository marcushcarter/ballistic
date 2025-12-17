#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class HierarchyPanel : public IPanel {
	public:
        HierarchyPanel(std::shared_ptr<ProjectManager> projectManager);
        
		void init() override;
		void OnImGuiRender() override;
		void onEvent(void* e) override;
    
    private:
        std::shared_ptr<ProjectManager> m_ProjectManager;
        
        struct HierarchyMetaData {
            std::vector<entt::entity> multiSelection;
            entt::entity lastClicked = entt::null;
            entt::entity shiftAnchor = entt::null;

            std::vector<entt::entity> deferredDestroy;
            entt::entity deferredDuplicate = entt::null;
        };

        HierarchyMetaData hierarchyMetadata;
        
        bool IsSelected(entt::entity e) const;
        void FlattenHierarchy(Scene& scene, entt::entity node, std::vector<entt::entity>& out);
        void DrawNode(Scene& scene, entt::entity e);
	};
}