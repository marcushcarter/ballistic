#pragma once

#include "Layer.h"

namespace Ballistic {

	class RenderLayer : public Layer {
	public:
		RenderLayer(const std::string name = "RenderLayer") : Layer(name) {}

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(void* e) override;
	private:
		
	};
}