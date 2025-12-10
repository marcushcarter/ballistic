#include <Ballistic.h>
#include <BallisticEntrypoint.h>
#include "EditorLayer.h"

namespace Ballistic {

	class BallisticEditor : public Application {
	public:
		BallisticEditor() {
			m_LayerStack->PushLayer(std::make_shared<EditorLayer>());
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