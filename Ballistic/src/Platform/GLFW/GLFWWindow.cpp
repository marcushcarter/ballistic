#include "GLFWWindow.h"
#include "Core/Config.h"

namespace Ballistic {

	GLFWWindow::GLFWWindow(const WindowProps& windowProps)
		: m_props(windowProps) {

		initGLFW();
		setupCallbacks();
	}

	GLFWWindow::~GLFWWindow() {
		if (m_window) glfwDestroyWindow(m_window);
	}

	void GLFWWindow::initGLFW() {
	    if (!glfwInit()) {
	        std::cerr << "Failed to initialize GLFW\n";
	        return;
	    }

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	    m_window = glfwCreateWindow(m_props.width, m_props.height, m_props.title.c_str(), nullptr, nullptr);
	    if (!m_window) {
	        std::cerr << "Failed to create GLFW window\n";
	        glfwTerminate();
	        return;
	    }
	
		glfwMakeContextCurrent(m_window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cerr << "Failed to initialize GLAD\n";
			glfwDestroyWindow(m_window);
			glfwTerminate();
			return;
		}

		glfwSwapInterval(m_props.vsync);
	}

	void GLFWWindow::setupCallbacks() {
		glfwSetWindowUserPointer(m_window, this);

		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* win, int w, int h) {
			auto window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(win));
			window->m_state.width = w;
			window->m_state.height = h;
			if (window->m_resizeCallback) window->m_resizeCallback(w, h);
		});

		glfwSetWindowFocusCallback(m_window, [](GLFWwindow* win, int focused) {
			auto window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(win));
			window->m_state.focused = focused != 0;
			if (window->m_focusCallback) window->m_focusCallback(focused != 0);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* win) {
			auto window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(win));
			if (window->m_closeCallback) window->m_closeCallback();
		});

	}

	bool GLFWWindow::ShouldClose() const { return glfwWindowShouldClose(m_window); }
	void GLFWWindow::PollEvents() { glfwPollEvents(); }
	void GLFWWindow::SwapBuffers() { glfwSwapBuffers(m_window); }

	void GLFWWindow::OnUpdate() {
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		m_state.width = w;
		m_state.height = h;

		m_state.focused = glfwGetWindowAttrib(m_window, GLFW_FOCUSED);
    	m_state.minimized = glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);

		float xscale, yscale;
		glfwGetWindowContentScale(m_window, &xscale, &yscale);
		m_state.dpiScale = xscale;
	}

	void GLFWWindow::ToggleFullscreen(bool fullscreen) {
		if (fullscreen == m_state.fullscreen) return;

	    if (fullscreen) {
	        glfwGetWindowPos(m_window, &m_state.windowedX, &m_state.windowedY);
	        glfwGetWindowSize(m_window, &m_state.windowedW, &m_state.windowedH);

	        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			
	        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			m_state.fullscreen = true;
	    } else {
	        glfwSetWindowMonitor(m_window, nullptr, m_state.windowedX, m_state.windowedY, m_state.windowedW, m_state.windowedH, 0);
			m_state.fullscreen = false;
	    }
	}
	
	void GLFWWindow::SetVSync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
		m_props.vsync = enabled;
	}

	void GLFWWindow::SetTitle(const std::string& title) {
		glfwSetWindowTitle(m_window, title.c_str());
		m_props.title = title;
    }

	void GLFWWindow::SetSize(int width, int height) { glfwSetWindowSize(m_window, width, height); }
	void GLFWWindow::SetPosition(int x, int y) { glfwSetWindowPos(m_window, x, y); }
	void GLFWWindow::Focus() { glfwFocusWindow(m_window); }
	void GLFWWindow::Minimize() { glfwIconifyWindow(m_window); }
	void GLFWWindow::Maximize(bool enabled) { glfwMaximizeWindow(m_window); }
}