#include "Core/Application.h"

namespace Ballistic {

	Application::Application() {
		_IWindow = Window::createWindow();
	}

	void Application::run() {

		while (!_IWindow->shouldClose()) {
			_IWindow->onUpdate();
		}
	}

	void Application::Shutdown() {}

};