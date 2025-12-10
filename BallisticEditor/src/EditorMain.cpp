#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic {

	class BallisticEditor : public Application {
	public:
		BallisticEditor() {
			m_LayerStack->PushLayer(std::make_shared<RenderLayer>());
		}

		void Shutdown() override {
			Application::Shutdown();
		}
		
		~BallisticEditor() {
		}

	};

	Application* CreateApplication() {
		return new BallisticEditor();
	}

};	