#include "Application.h"

#include "Layers/RenderLayer.h"
#include "Platform/Vulkan/VulkanTest.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {
		m_Window = GLFWWindow::CreateWindow(windowProps);

		auto renderLayer = std::make_shared<RenderLayer>(m_LayerStack, "RenderLayer");
		m_LayerStack.pushLayer(renderLayer);
		m_RenderLayer = renderLayer;

		VulkanTest();
	}

	void Application::Shutdown(){
	}

	void Application::run() {
		while (!m_Window->shouldClose()) {
			m_LayerStack.onUpdate();

			m_Window->onUpdate();
		}

		Shutdown();
	}
}