#include "IWindow.h"
#include "WindowInfo.h"

#include "Platform/GLFW/GLFWWindow.h"

namespace Ballistic {
	WindowAPI::API WindowAPI::s_api = WindowAPI::API::GLFW;

	IWindow::WindowPtr IWindow::Create(const WindowProps& windowProps) {
		#ifdef _WIN32
			std::cout << "Chose GLFW for Windows\n";
			WindowAPI::SetAPI(WindowAPI::API::GLFW);
			return std::make_shared<GLFWWindow>(windowProps);
		#elif defined(__APPLE__)
			std::cout << "Chose None for Apple\n";
		#elif defined(__linux__)
			std::cout << "Chose None for Linux\n";
		#else
			std::cout << "Chose GLFW for Unknown Platform\n";
			WindowAPI::SetAPI(WindowAPI::API::GLFW);
			return std::make_shared<GLFWWindow>(windowProps);
		#endif
	}
}