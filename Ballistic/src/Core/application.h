#pragma once

#include "lrpch.h"
#include "Platform/Windows/GLFWWindow.h"

namespace Ballistic {

	class Application {
	public:
		Application();
		virtual ~Application() = default;
		void run();

	private:

		std::shared_ptr<GLFWWindowIMPL> _IWindow;
	};

};	