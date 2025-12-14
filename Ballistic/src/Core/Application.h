#pragma once

#include "bepch.h"
#include "Core/IWindow.h"
#include "Layers/LayerStack.h"
#include "Renderer/VkRenderer.h"

namespace Ballistic {

	class RenderLayer;
	class EditorLayer;
	class RuntimeLayer;

	struct LayerContext {
	    LayerStack* layerStack;
	    IWindow* window;
	    VkRenderer* renderer;
	};

	class Application {
	public:
		Application(WindowProps windowProps = WindowProps{});
		virtual ~Application() = default;
		void run();

	protected:
		LayerStack m_LayerStack;

		std::shared_ptr<IWindow> m_Window;

		std::weak_ptr<RenderLayer> m_RenderLayer;
		std::unique_ptr<VkRenderer> m_VkRenderer;

		virtual void Shutdown();
	};

	Application* CreateApplication();
}