#pragma once

#include "bepch.h"
// include event

namespace Ballistic {

	class Layer {
	public:
		Layer(const std::string name = "Layer") : m_Name(name) {
			std::cout << m_Name << " Attached" << std::endl;
		}

		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(void* e) {}

		const std::string& getName() const { return m_Name; }

	private:
		std::string m_Name;
	};
}