#include "Platform/Windows/GLFWWindow.h"

namespace Ballistic {

	GLFWWindowIMPL::GLFWWindowIMPL(const WindowProps& windowProps) {

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

	    // glfwSetKeyCallback(m_NativeWindow, glfwKeyCallback);

	    // GLFWimage image;
	    // image.pixels = stbi_load("app/resources/textures/favicon.png", &image.width, &image.height, nullptr, 4);
	    // if (image.pixels) {
	    //     glfwSetWindowIcon(m_NativeWindow, 1, &image);
	    //     stbi_image_free(image.pixels);
	    // } else {
	    //     std::cout << "Failed to set window icon" << std::endl;
	    // }

	    glfwMakeContextCurrent(m_NativeWindow);

	    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	        if (!m_NativeWindow) {
	            std::cout << "Failed to initialize GLAD" << std::endl;
	            glfwDestroyWindow(m_NativeWindow);
	            glfwTerminate();
	        }
	    }

	    // std::cout << glGetString(GL_VERSION) << std::endl;
	    glfwSwapInterval(windowProps.VSync);

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

	std::shared_ptr<GLFWWindowIMPL> GLFWWindowIMPL::createWindow(const WindowProps windowProps) {
		return std::make_shared<GLFWWindowIMPL>(windowProps);
	}

};