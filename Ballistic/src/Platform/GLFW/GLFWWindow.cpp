#include "GLFWWindow.h"

namespace Ballistic {

	GLFWWindow::GLFWWindow(const WindowProps& windowProps)
		: m_Props(windowProps) {

	    if (!glfwInit()) {
	        std::cerr << "Failed to initialize GLFW" << std::endl;
	        glfwTerminate();
	        return;
	    }

	    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	    m_NativeWindow = glfwCreateWindow(windowProps.width, windowProps.height, (windowProps.title).c_str(), nullptr, nullptr);
	    if (!m_NativeWindow) {
	        std::cerr << "Failed to create GLFW window" << std::endl;
	        glfwTerminate();
	        return;
	    }
	}

	GLFWWindow::~GLFWWindow() {
		if (m_NativeWindow) glfwDestroyWindow(m_NativeWindow);
		glfwTerminate();
	}

	void GLFWWindow::onUpdate() {
		glfwPollEvents();
		glfwSwapBuffers(m_NativeWindow);
	}

	bool GLFWWindow::shouldClose() const {
		return glfwWindowShouldClose(m_NativeWindow);
	}

	void GLFWWindow::toggleFullscreen(bool fullscreen) {
		if (fullscreen == isFullscreen()) return;

	    if (fullscreen) {
	        glfwGetWindowPos(m_NativeWindow, &m_WindowedX, &m_WindowedY);
	        glfwGetWindowSize(m_NativeWindow, &m_WindowedW, &m_WindowedH);

	        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	        glfwSetWindowMonitor(m_NativeWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	    } else {
	        glfwSetWindowMonitor(m_NativeWindow, nullptr, m_WindowedX, m_WindowedY, m_WindowedW, m_WindowedH, 0);
	    }
	}

	bool GLFWWindow::isFullscreen() const {
		return glfwGetWindowMonitor(m_NativeWindow) != nullptr;
	}

	std::shared_ptr<IWindow> GLFWWindow::Create(const WindowProps& windowProps) {
		return std::make_shared<GLFWWindow>(windowProps);
	}
}