#include "Application.h"
#include "Project/ProjectManager.h"
#include "Layers/LayerStack.h"
#include "Layers/RenderLayer.h"
#include "Platform/GLFW/GLFWWindow.h"
#include "Renderer/IRenderer.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {

		m_layerStack = std::make_shared<LayerStack>();

		m_window = IWindow::Create(windowProps);

		m_projectManager = std::make_shared<ProjectManager>();

	    m_renderer = std::make_shared<IRenderer>();
		m_renderer->Init();

		m_layerStack->PushLayer(std::make_shared<RenderLayer>(GetAppContext(), "RenderLayer"));
	}

	void Application::Shutdown(){
		if (m_renderer)
	        m_renderer->Shutdown();
	}

	void Application::Run() {
		while (!m_window->ShouldClose()) {
			m_window->PollEvents();
			m_window->OnUpdate();

			m_layerStack->OnUpdate();
			
			m_window->SwapBuffers();
		}
		Shutdown();
	}
}