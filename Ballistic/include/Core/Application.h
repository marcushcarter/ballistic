#pragma once
#include "bepch.h"
#include "Window/WindowInfo.h"

namespace Ballistic {

	class ProjectManager;
	class LayerStack;
	class Window;
	class IRenderer;

	struct LayerContext {
		std::shared_ptr<ProjectManager> projectManager;
	    std::shared_ptr<LayerStack> layerStack;
	    std::shared_ptr<Window> window;
	    std::shared_ptr<IRenderer> renderer;
	};

	class Application {
	public:
		Application(WindowProps windowProps = WindowProps{});
		virtual ~Application() = default;
		void Run();

		LayerContext GetAppContext() {
			LayerContext context;
			context.projectManager = m_projectManager;
            context.layerStack = m_layerStack;
            context.window = m_window;
            context.renderer = m_renderer;
			return context;
		}

	protected:
		std::shared_ptr<ProjectManager> m_projectManager;
		std::shared_ptr<LayerStack> m_layerStack;
		std::shared_ptr<Window> m_window;
		std::shared_ptr<IRenderer> m_renderer;

		virtual void Shutdown() = 0;
	};

	Application* CreateApplication(const std::filesystem::path& exeDir);
}