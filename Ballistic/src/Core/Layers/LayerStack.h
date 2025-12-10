#pragma once

#include "lrpch.h"
#include "Layer.h"

namespace Ballistic {

	class LayerStack {
	public:
		LayerStack() = default;
	    ~LayerStack();

	    void PushLayer(std::shared_ptr<Layer> layer);
	    void PopLayer(std::shared_ptr<Layer> layer);

	    void OnUpdate();
	    void DispatchEvent(void* e);

	private:
	    std::vector<std::shared_ptr<Layer>> m_Layers;
	};
}