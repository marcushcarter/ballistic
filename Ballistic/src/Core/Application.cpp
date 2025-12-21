#include "Core/Application.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Layers/RenderLayer.h"
#include "Core/Window/Window.h"
#include "Project/ProjectManager.h"
#include "Renderer/IRenderer.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {

		m_layerStack = std::make_shared<LayerStack>();

		m_window = std::make_shared<Window>(windowProps);

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
			Window::PollEvents();
			m_window->OnUpdate();

			m_layerStack->OnUpdate();
			
			m_window->SwapBuffers();
		}
		Shutdown();
	}
}