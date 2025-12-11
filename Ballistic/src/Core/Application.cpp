#include "Application.h"

namespace Ballistic {

	Application::Application(WindowProps windowProps) {
		m_Window = GLFWWindow::CreateWindow(windowProps);
	}

	void Application::Shutdown(){
	}

	void Application::run() {
		while (!m_Window->shouldClose()) {

			m_Window->onUpdate();
		}

		Shutdown();
	}

}