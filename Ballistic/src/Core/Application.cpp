#include "Application.h"

#include "Layers/RenderLayer.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {
		m_Window = GLFWWindow::CreateWindow(windowProps);

		auto renderLayer = std::make_shared<RenderLayer>(m_LayerStack, "RenderLayer");
		m_LayerStack.pushLayer(renderLayer);
		m_RenderLayer = renderLayer;

		m_Renderer = std::make_unique<VulkanRenderer>();
		m_Renderer->Init();
	}

	void Application::Shutdown(){
		m_Renderer->Shutdown();
	}

	void Application::run() {
		while (!m_Window->shouldClose()) {
			m_LayerStack.onUpdate();

			m_Window->onUpdate();
		}

		Shutdown();
	}
}