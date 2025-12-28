#pragma once
#include <Ballistic.h>

namespace ballistic
{
    class IPanel {
    public:
        IPanel(const std::string name = "Panel")
            : m_name(name), m_open(true) {}
        
        virtual ~IPanel() = default;

        void Attach();
        void Detach();

        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnEvent(IEvent& e) = 0;

        bool IsOpen() const { return m_open; }
        void SetOpen(bool open) { m_open = open; }

		const std::string& getName() const { return m_name; }
        
	protected:
		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;

        bool m_open;
    
    private:
		std::string m_name;
    };
    
} // namespace ballistic
