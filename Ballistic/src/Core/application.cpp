#include "Application.h"

#include "Layer/LayerStack.h"

namespace Ballistic {

	Application::Application() {
		m_Window = Window::createWindow();
		m_LayerStack = std::make_shared<LayerStack>();
	}

	void Application::run() {

		while (!m_Window->shouldClose()) {

			float dt = 0.016f;

			// for (Layer* layer : m_LayerStack)
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