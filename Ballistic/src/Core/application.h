#pragma once

#include "lrpch.h"
#include "Platform/GLFW/Window.h"

namespace Ballistic {
	class Layer;
	class LayerStack;
	class RenderLayer;
}

namespace Ballistic {

	class Application {
	public:
		Application();
		virtual ~Application() = default;
		
		void run();
		virtual void Shutdown();

	protected:
		std::shared_ptr<Window> m_Window;

    	std::shared_ptr<LayerStack> m_LayerStack;

    	std::shared_ptr<RenderLayer> m_RenderLayer;
	};

	Application* CreateApplication();

};	