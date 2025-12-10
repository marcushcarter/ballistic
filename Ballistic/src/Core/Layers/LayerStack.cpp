#include "LayerStack.h"

namespace Ballistic {

	LayerStack::~LayerStack() {
	    for (auto& layer : m_Layers)
	        layer->OnDetach();
	}

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
	    m_Layers.push_back(layer);
	    layer->OnAttach();
	}

	void LayerStack::PopLayer(std::shared_ptr<Layer> layer) {
	    auto it = std::remove(m_Layers.begin(), m_Layers.end(), layer);
	    if (it != m_Layers.end()) {
	        (*it)->OnDetach();
	        m_Layers.erase(it, m_Layers.end());
	    }
	}

	void LayerStack::OnUpdate() {
	    for (auto& layer : m_Layers)
	        layer->OnUpdate();
	}

	void LayerStack::DispatchEvent(void* e) {
	    for (auto& layer : m_Layers)
	        layer->OnEvent(e);
	}


}