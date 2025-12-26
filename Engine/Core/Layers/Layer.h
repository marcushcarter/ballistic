#pragma once
#include "bepch.h"

namespace Ballistic {

	class Layer {
	public:
		Layer(const std::string name = "Layer")
			: m_name(name) {
			std::cout << m_name << " Attached" << std::endl;
		}

		virtual ~Layer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEvent(void* ePtr) = 0;

		const std::string& getName() const { return m_name; }

	private:
		std::string m_name;
	};
}