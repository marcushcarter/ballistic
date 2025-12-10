#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "RuntimeLayer.h"

namespace Ballistic {

	class BallisticRuntime : public Application {
	public:
		BallisticRuntime() {
			m_LayerStack->PushLayer(std::make_shared<RuntimeLayer>());
		}
		
		~BallisticRuntime() {
			
		}

	};

	Application* CreateApplication() {
		return new BallisticRuntime();
	}

};	