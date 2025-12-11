#pragma once

#include "bepch.h"
#include "Platform/GLFW/GLFWWindow.h"

namespace Ballistic {

	class Application {
	public:
		Application(WindowProps windowProps = WindowProps{});
		virtual ~Application() = default;
		void run();

	protected:
		std::shared_ptr<GLFWWindow> m_Window;

		virtual void Shutdown();
	};

	Application* CreateApplication();

}