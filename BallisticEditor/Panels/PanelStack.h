#pragma once
#include <Ballistic.h>

namespace ballistic
{
	class IPanel;

	class PanelStack {
	public:
		PanelStack() = default;
		~PanelStack() { OnDetach(); }

		void PushPanel(std::shared_ptr<IPanel> panel);
		void PopPanel(std::shared_ptr<IPanel> panel);

		void OnUpdate(float deltaTime);
		void OnDetach();

	public:
		std::vector<std::shared_ptr<IPanel>> m_panels;
	};
    
} // namespace ballistic
