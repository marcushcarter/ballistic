#include "Core/Application.h"
#include "Core/Layers/LayerStack.h"
#include "Core/Layers/RenderLayer.h"
#include "Core/Window/Window.h"
#include "Project/ProjectManager.h"
#include "Renderer/Renderer.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {

		Window::InitGLFW();

		m_window = std::make_shared<Window>(windowProps);

		m_projectManager = std::make_shared<ProjectManager>();

	    m_renderer = std::make_shared<Renderer>();
		m_renderer->Init();

		m_layerStack = std::make_shared<LayerStack>();
		m_layerStack->PushLayer(std::make_shared<RenderLayer>(GetAppContext(), "RenderLayer"));
	}

	void Application::Shutdown(){
		if (m_renderer)
	        m_renderer->Shutdown();
	}

	void Application::Run() {
		while (!m_window->ShouldClose()) {
			Window::PollEvents();

			gl::ClearColor(0.0, 0.0, 0.0, 1.0);
			gl::Clear();

			// ===== Window =====

			m_window->BeginFrame();

			m_layerStack->OnUpdate();
			
			m_window->EndFrame();
		}
		Shutdown();
	}
}