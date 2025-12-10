#pragma once

#include "lrpch.h"
#include "Platform/GLFW/Window.h"

namespace Ballistic {

	class Application {
	public:
		Application();
		virtual ~Application() = default;
		void run();

	protected:

		std::shared_ptr<Window> _IWindow;

		virtual void Shutdown();
	};

	Application* CreateApplication();

};	