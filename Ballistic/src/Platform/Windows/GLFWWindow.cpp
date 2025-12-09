#include "Platform/Windows/GLFWWindow.h"

namespace Ballistic {

	GLFWWindowIMPL::GLFWWindowIMPL(const WindowProps& windowProps) {
		if (!glfwInit()) {}

		m_NativeWindow = glfwCreateWindow(windowProps.width, windowProps.height, (windowProps.title).c_str(), nullptr, nullptr);
		if (!m_NativeWindow) {}

		glfwMakeContextCurrent(m_NativeWindow);

		glfwSwapInterval(1);

	}

	GLFWWindowIMPL::~GLFWWindowIMPL() {
		if (m_NativeWindow) glfwDestroyWindow(m_NativeWindow);
		glfwTerminate();
	}

	void GLFWWindowIMPL::onUpdate() {
		glfwPollEvents();
		glfwSwapBuffers(m_NativeWindow);
	}

	bool GLFWWindowIMPL::shouldClose() {
		return glfwWindowShouldClose(m_NativeWindow);
	}

};