#pragma once

#include "bepch.h"
#include "Platform/GLFW/GLFWWindow.h"
#include "Layers/LayerStack.h"

namespace Ballistic {

	class RenderLayer;
	class EditorLayer;
	class RuntimeLayer;

	class Application {
	public:
		Application(WindowProps windowProps = WindowProps{});
		virtual ~Application() = default;
		void run();

	protected:
		std::shared_ptr<GLFWWindow> m_Window;

		LayerStack m_LayerStack;
		std::weak_ptr<RenderLayer> m_RenderLayer;

		virtual void Shutdown();
	};

	Application* CreateApplication();
}