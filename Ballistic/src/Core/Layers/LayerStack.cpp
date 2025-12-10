#include "LayerStack.h"

namespace Ballistic {

	LayerStack::~LayerStack() {
	    for (std::shared_ptr<Layer>& layer : m_Layers)
	        layer->OnDetach();
	}

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
	    m_Layers.push_back(layer);
	    layer->OnAttach();
	}

	void LayerStack::PopLayer(std::shared_ptr<Layer> layer) {
        m_Layers.erase(std::remove(m_Layers.begin(), m_Layers.end(), layer), m_Layers.end());
        layer->OnDetach();
	}

	void LayerStack::OnUpdate() {
	    for (std::shared_ptr<Layer>& layer : m_Layers)
	        layer->OnUpdate();
	}

	// void LayerStack::DispatchEvent(void* e) {
	//     for (std::shared_ptr<Layer>& layer : m_Layers)
	//         layer->OnEvent(e);
	// }

	void LayerStack::DispatchEvent(std::shared_ptr<IEvent> event) {
	    for (std::shared_ptr<Layer>& layer : m_Layers) {
	        layer->OnEvent(event);
	        // if (event->IsConsumed())
	        // 	break;
	    }
	}


}