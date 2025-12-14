#include "Application.h"

#include "Layers/RenderLayer.h"

#include "Platform/GLFW/GLFWWindow.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {

		WindowAPI::SetAPI(WindowAPI::API::GLFW);

		switch (WindowAPI::GetAPI()) {
			case WindowAPI::API::GLFW:
				m_Window = GLFWWindow::Create(windowProps);
				break;
		}

	    m_VkRenderer = std::make_unique<VkRenderer>();
		m_VkRenderer->Init();

		auto renderLayer = std::make_shared<RenderLayer>(m_LayerStack, "RenderLayer");
		m_LayerStack.pushLayer(renderLayer);
		m_RenderLayer = renderLayer;
	}

	void Application::Shutdown(){
		 if (m_VkRenderer)
	        m_VkRenderer->Shutdown();
	}

	void Application::run() {
		while (!m_Window->shouldClose()) {
			m_LayerStack.onUpdate();

			m_Window->onUpdate();
		}

		Shutdown();
	}
}