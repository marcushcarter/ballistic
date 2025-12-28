#include "Core/Window/Window.h"
#include "Core/Window/WindowInfo.h"

#include <stb_image.h>
#include "Core/LogManager/Log.h"

namespace ballistic
{
    Window::Window() {
        if (!glfwInit()) {
            LogError("Failed to initialize GLFW");
            return;
        }

        glfwWindowHint(GLFW_RESIZABLE, m_settings.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, m_settings.customTitleBar ? GLFW_FALSE : GLFW_TRUE);
    }

    bool Window::Init(const WindowSettings& windowSettings) {
        m_settings = windowSettings;

        m_nativeWindow = glfwCreateWindow(m_settings.width, m_settings.height, m_settings.title.c_str(), nullptr, nullptr);
        if (!m_nativeWindow) {
            LogError("Failed to create window");
            Shutdown();
            return false;
        }

		glfwSwapInterval(m_settings.vsync ? 1 : 0);

		glfwSetWindowUserPointer(m_nativeWindow, this);

		glfwSetFramebufferSizeCallback(m_nativeWindow, [](GLFWwindow* win, int w, int h) {
			auto window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			window->m_state.width = w;
			window->m_state.height = h;
			if (window->m_resizeCallback) window->m_resizeCallback(w, h);
		});

		glfwSetWindowFocusCallback(m_nativeWindow, [](GLFWwindow* win, int focused) {
			auto window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			window->m_state.focused = focused != 0;
			if (window->m_focusCallback) window->m_focusCallback(focused != 0);
		});

		glfwSetWindowCloseCallback(m_nativeWindow, [](GLFWwindow* win) {
			auto window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			if (window->m_closeCallback) window->m_closeCallback();
		});

        return true;
    }

    bool Window::ShouldClose() const {
        return m_nativeWindow && glfwWindowShouldClose(m_nativeWindow);
    }

    void Window::Update(float deltaTime) {
        if (!m_nativeWindow) return;

        int w, h;
		glfwGetWindowSize(m_nativeWindow, &w, &h);
		m_state.width = w;
		m_state.height = h;

        m_state.focused = glfwGetWindowAttrib(m_nativeWindow, GLFW_FOCUSED);
        m_state.minimized = glfwGetWindowAttrib(m_nativeWindow, GLFW_ICONIFIED);
        m_state.maximized = glfwGetWindowAttrib(m_nativeWindow, GLFW_MAXIMIZED);

		float xscale, yscale;
		glfwGetWindowContentScale(m_nativeWindow, &xscale, &yscale);
		m_state.dpiScale = xscale;

        glfwPollEvents();        
        glfwSwapBuffers(m_nativeWindow);

    }

    void Window::Shutdown() {
        if (m_nativeWindow) {
            glfwDestroyWindow(m_nativeWindow);
            m_nativeWindow = nullptr;
        }
        glfwTerminate();
    }

    bool Window::SetIcon(const std::filesystem::path& path) {
        int w, h, c;
        if (unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &c, 4)) {
            GLFWimage icon{ w, h, data };
            glfwSetWindowIcon(m_nativeWindow, 1, &icon);
            stbi_image_free(data);
            return true;
        }
        LogError("Failed to load icon: ", path);
        return false;
    }

    bool Window::SetIcon(const void* pixels, int width, int height, int channels) {
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = const_cast<unsigned char*>(static_cast<const unsigned char*>(pixels));
        glfwSetWindowIcon(m_nativeWindow, 1, &icon);
        return true;
    }

    void Window::ToggleFullscreen(bool fullscreen) {
		if (fullscreen == m_state.fullscreen) return;

	    if (fullscreen) {
	        glfwGetWindowPos(m_nativeWindow, &m_state.windowedX, &m_state.windowedY);
	        glfwGetWindowSize(m_nativeWindow, &m_state.windowedW, &m_state.windowedH);

	        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			
	        glfwSetWindowMonitor(m_nativeWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			m_state.fullscreen = true;
	    } else {
	        glfwSetWindowMonitor(m_nativeWindow, nullptr, m_state.windowedX, m_state.windowedY, m_state.windowedW, m_state.windowedH, 0);
			m_state.fullscreen = false;
	    }
	}
	
	void Window::SetVSync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
		m_settings.vsync = enabled;
	}

	void Window::SetTitle(const std::string& title) {
        if (title == m_settings.title) return;
		glfwSetWindowTitle(m_nativeWindow, title.c_str());
		m_settings.title = title;
    }

	void Window::SetSize(int width, int height) { glfwSetWindowSize(m_nativeWindow, width, height); }
	void Window::SetPosition(int x, int y) { glfwSetWindowPos(m_nativeWindow, x, y); }
	void Window::Focus() { glfwFocusWindow(m_nativeWindow); }
	void Window::Minimize() { glfwIconifyWindow(m_nativeWindow); }
	void Window::Maximize(bool enabled) { if (enabled) glfwMaximizeWindow(m_nativeWindow); else glfwRestoreWindow(m_nativeWindow); }

} // namespace ballistic
