#pragma once
#include "bepch.h"

namespace ballistic
{
	class ILayer
    {
	public:
		ILayer(const std::string name = "ILayer") : m_name(name) {
			std::cout << m_name << " Attached" << std::endl;
		}

		virtual ~ILayer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnEvent(IEvent& e) = 0;
		virtual void OnUIRender() = 0;

		const std::string& getName() const { return m_name; }

	private:
		std::string m_name;
	};
    
} // namespace ballistic