#pragma once

#include "lrpch.h"
#include "Layer.h"

namespace Ballistic {

	class LayerStack {
	public:
		LayerStack() = default;
	    ~LayerStack();
	    
	    std::vector<std::shared_ptr<Layer>> m_Layers;

	    void PushLayer(std::shared_ptr<Layer> layer);
	    void PopLayer(std::shared_ptr<Layer> layer);

	    void OnUpdate();
	    void DispatchEvent(void* e);

	    auto begin() { return m_Layers.begin(); }
	    auto end() { return m_Layers.end(); }
	    auto begin() const { return m_Layers.begin(); }
	    auto end() const { return m_Layers.end(); }

	private:
	    // std::vector<std::shared_ptr<Layer>> m_Layers;
	};
}