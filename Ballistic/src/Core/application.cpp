#include "Application.h"

#include "Layers/LayerStack.h"
#include "Layers/RenderLayer.h"

namespace Ballistic {

	Application::Application() {
		m_Window = Window::createWindow();
		m_LayerStack = std::make_shared<LayerStack>();

		m_RenderLayer = std::make_shared<RenderLayer>(m_LayerStack /*_Profiler, _ProjectManager*/);
		m_LayerStack->PushLayer(m_RenderLayer);
	}

	void Application::run() {

		while (!m_Window->shouldClose()) {

			float dt = 0.016f;

	    	m_LayerStack->OnUpdate();

			m_Window->onUpdate();
		}
	}

	void Application::Shutdown() {}

};