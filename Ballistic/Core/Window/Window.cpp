#include "Core/Window/Window.h"
#include "Core/Window/WindowInfo.h"

namespace ballistic
{
    bool Window::Init(const WindowSettings& windowSettings)
    {
        m_settings = windowSettings;

        if (!glfwInit())
            return false;
            
        glfwWindowHint(GLFW_RESIZABLE, m_settings.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, m_settings.customTitleBar ? GLFW_FALSE : GLFW_TRUE);

        m_nativeWindow = glfwCreateWindow(
            m_settings.width,
            m_settings.height,
            m_settings.title.c_str(),
            nullptr, nullptr
        );

        if (!m_nativeWindow) {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_nativeWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            Shutdown();
            return false;
        }

        glfwSetWindowUserPointer(m_nativeWindow, this); 

        glfwSetMouseButtonCallback(m_nativeWindow, [](GLFWwindow* wnd, int button, int action, int mods){
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(wnd));
            if (!window || !window->m_settings.customTitleBar) return;

            double mx, my;
            glfwGetCursorPos(wnd, &mx, &my);

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS && my < 30) { // top 30px = title bar
                    window->m_dragging = true;
                    window->m_dragOffsetX = mx;
                    window->m_dragOffsetY = my;
                } else if (action == GLFW_RELEASE) {
                    window->m_dragging = false;
                }
            }
        });


        return true;
    }

    bool Window::ShouldClose() const {
        return m_nativeWindow && glfwWindowShouldClose(m_nativeWindow);
    }

    void Window::Update() {
        if (!m_nativeWindow) return;
        glfwPollEvents();

        if (m_settings.customTitleBar) {
            double mx, my;
            glfwGetCursorPos(m_nativeWindow, &mx, &my);

            if (m_dragging) {
                int newX = int(mx - m_dragOffsetX);
                int newY = int(my - m_dragOffsetY);
                glfwSetWindowPos(m_nativeWindow, newX, newY);
            }
        }
        
        glfwSwapBuffers(m_nativeWindow);

    }

    void Window::Shutdown() {
        if (m_nativeWindow) {
            glfwDestroyWindow(m_nativeWindow);
            m_nativeWindow = nullptr;
        }
        glfwTerminate();
    }

} // namespace ballistic
