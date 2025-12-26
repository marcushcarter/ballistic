#pragma once
#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace Ballistic {

	class MenuBar : public IPanel {
	public:
        MenuBar(std::shared_ptr<ProjectManager> projectManager, std::shared_ptr<Window> window);
        
		void Init() override;
		void OnImGuiRender() override;
		void OnEvent(void* ePtr) override;

		void OpenLink(const std::string& url);

    private:
        std::shared_ptr<ProjectManager> m_projectManager;
        std::shared_ptr<Window> m_window;
	};
}