#pragma once

#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class InspectorPanel : public IPanel {
	public:
        InspectorPanel(std::shared_ptr<ProjectManager> projectManager);
        
		void init() override;
		void OnImGuiRender() override;
		void onEvent(void* e) override;
    
    private:
        std::shared_ptr<ProjectManager> m_ProjectManager;
	};
}