#pragma once
#include <Ballistic.h>
#include "Panels/IPanel.h"

namespace ballistic
{
	class IPanel;

	enum class PanelStaction {
		None,
		OpenEditor,
		OpenLauncher
	};

	class PanelStack {
	public:
		PanelStack(LayerContext& layerContext);
		~PanelStack() { Clear(); }

		void PushPanel(std::unique_ptr<IPanel> panel);
		void PopPanel(IPanel* panel);

		void OnUpdate(float deltaTime);
		void Clear();
		void DispatchEvent(IEvent& e);

		void OpenEditor(LayerContext& context);
		void OpenLauncher(LayerContext& context);

		void QueueAction(PanelStaction action) { m_pendingAction = action; }

	private:
		std::vector<std::unique_ptr<IPanel>> m_panels;
		PanelStaction m_pendingAction = PanelStaction::None;
		LayerContext& m_context;

		void ExecutePendingAction();
	};
    
} // namespace ballistic
