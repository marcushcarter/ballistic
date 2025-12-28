#include "Core/Window/Window.h"
#include "Core/Window/WindowInfo.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace ballistic
{
    bool Window::Init(const WindowSettings& windowSettings) {
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

        return true;
    }

    bool Window::ShouldClose() const {
        return m_nativeWindow && glfwWindowShouldClose(m_nativeWindow);
    }

    void Window::Update(float deltaTime) {
        if (!m_nativeWindow) return;
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

} // namespace ballistic
