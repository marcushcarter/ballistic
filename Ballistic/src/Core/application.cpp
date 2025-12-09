#include "Core/Application.h"
#include "Core/IWindow.h"

namespace Ballistic {

	Application::Application() {
		_IWindow = IWindow::createWindow();
	}

	void Application::run() {
		std::cout << "Hello, World!" << std::endl;

		while (!_IWindow->shouldClose()) {
			_IWindow->onUpdate();
		}
	}

};	