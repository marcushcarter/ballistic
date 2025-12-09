#include "Core/Application.h"

namespace Ballistic {

	Application::Application() {
		_IWindow = GLFWWindowIMPL::createWindow();
	}

	void Application::run() {

		while (!_IWindow->shouldClose()) {
			_IWindow->onUpdate();
		}
	}

};	