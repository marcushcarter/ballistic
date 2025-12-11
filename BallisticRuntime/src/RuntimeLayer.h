#pragma once

#include <Ballistic.h>

namespace Ballistic {

	class RuntimeLayer : public Layer {
	public:
		RuntimeLayer(const std::string name = "RuntimeLayer") : Layer(name) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;
	private:
		
	};
}