#pragma once
#include "bepch.h"
#include "Renderer/OglRenderer.h"

namespace Ballistic {

	class LayerStack;
	class RenderLayer;
	class EditorLayer;
	class RuntimeLayer;
	class ProjectManager;

	struct LayerContext {
	    std::shared_ptr<LayerStack> layerStack;
	    std::shared_ptr<IWindow> window;
	    std::shared_ptr<OglRenderer> renderer;
		std::shared_ptr<ProjectManager> projectManager;
	};

	class Application {
	public:
		Application(WindowProps windowProps = WindowProps{});
		virtual ~Application() = default;
		void run();

	protected:
		std::shared_ptr<ProjectManager> m_ProjectManager;

		std::shared_ptr<LayerStack> m_LayerStack;
		std::weak_ptr<RenderLayer> m_RenderLayer;

		std::shared_ptr<IWindow> m_Window;
		std::shared_ptr<OglRenderer> m_OglRenderer;

		virtual void Shutdown();
	};

	Application* CreateApplication(const std::filesystem::path& exeDir);
}