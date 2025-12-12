#include "GLFWWindow.h"

namespace Ballistic {

	GLFWWindow::GLFWWindow(const WindowProps& windowProps) {

	    if (!glfwInit()) {
	        std::cerr << "Failed to initialize GLFW" << std::endl;
	        glfwTerminate();
	    }

	    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	    m_NativeWindow = glfwCreateWindow(windowProps.width, windowProps.height, (windowProps.title).c_str(), nullptr, nullptr);
	    if (!m_NativeWindow) {
	        std::cerr << "Failed to create GLFW window" << std::endl;
	        glfwDestroyWindow(m_NativeWindow);
	        glfwTerminate();
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

	std::shared_ptr<GLFWWindow> GLFWWindow::CreateWindow(const WindowProps& windowProps) {
		return std::make_shared<GLFWWindow>(windowProps);
	}
}