#include "Application.h"
#include "Project/ProjectManager.h"
#include "Layers/LayerStack.h"
#include "Layers/RenderLayer.h"
#include "Platform/GLFW/GLFWWindow.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {

		m_LayerStack = std::make_shared<LayerStack>();

		WindowAPI::SetAPI(WindowAPI::API::GLFW);
		switch (WindowAPI::GetAPI()) {
			case WindowAPI::API::GLFW:
				m_Window = GLFWWindow::Create(windowProps);
				break;
		}

		m_ProjectManager = std::make_shared<ProjectManager>();

	    m_OglRenderer = std::make_shared<OglRenderer>(m_Window);
		m_OglRenderer->Init();

		auto renderLayer = std::make_shared<RenderLayer>(m_LayerStack, "RenderLayer");
		m_LayerStack->pushLayer(renderLayer);
		m_RenderLayer = renderLayer;
	}

	void Application::Shutdown(){
		 if (m_OglRenderer)
	        m_OglRenderer->Shutdown();
	}

	void Application::run() {
		while (!m_Window->shouldClose()) {
			
			m_LayerStack->onUpdate();
			
			m_OglRenderer->Render();

			m_Window->onUpdate();
		}

		Shutdown();
	}
}