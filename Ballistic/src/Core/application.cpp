#include "Application.h"

#include "Layers/LayerStack.h"
#include "Layers/RenderLayer.h"

namespace Ballistic {

	Application::Application() {
		m_Window = Window::createWindow();
		m_LayerStack = std::make_shared<LayerStack>();

		m_RenderLayer = std::make_shared<RenderLayer>(/*_LayerStack, _Profiler, _ProjectManager*/);
		m_LayerStack->PushLayer(m_RenderLayer);
	}

	void Application::run() {

		while (!m_Window->shouldClose()) {

			float dt = 0.016f;

	    	m_LayerStack->OnUpdate();

			// for (auto& layer : m_LayerStack->m_Layers)
            //     layer->OnUpdate();

            // // m_Window->beginImGui();

            // for (Layer* layer : m_LayerStack)
            //     layer->OnImGuiRender();

            // m_Window->endImGui();

			m_Window->onUpdate();
		}
	}

	void Application::Shutdown() {}

};