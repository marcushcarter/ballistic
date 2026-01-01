#pragma once
#include <Ballistic.h>
#include "Panels/PanelStack.h"

namespace ballistic
{
    class PanelStack;
    
    class IPanel {
    public:
        IPanel(LayerContext context, PanelStack& panelStack, const std::string name = "Panel")
            : m_context(context), m_panelStack(panelStack), m_name(name), m_open(true) {}
        
        virtual ~IPanel() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnEvent(IEvent& e) = 0;

        bool IsOpen() const { return m_open; }
        void SetOpen(bool open) { m_open = open; }

		const std::string& getName() const { return m_name; }
        
	protected:
        LayerContext m_context;
        PanelStack& m_panelStack;
        bool m_open;
    
    private:
		std::string m_name;
    };
    
} // namespace ballistic
