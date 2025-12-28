#include "Core/Window/Window.h"
#include "Core/Window/WindowInfo.h"

#include <stb_image.h>
#include "Core/Logging/Log.h"

namespace ballistic
{
    bool Window::Init(const WindowSettings& windowSettings) {
        m_settings = windowSettings;

        if (!glfwInit()) {
            LogFatal("Failed to initialize GLFW");
            return false;
        }
            
        glfwWindowHint(GLFW_RESIZABLE, m_settings.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, m_settings.customTitleBar ? GLFW_FALSE : GLFW_TRUE);

        m_nativeWindow = glfwCreateWindow(
            m_settings.width,
            m_settings.height,
            m_settings.title.c_str(),
            nullptr, nullptr
        );

        if (!m_nativeWindow) {
            LogFatal("Failed to open window");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_nativeWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LogFatal("Failed to initialize GLAD");
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

} // namespace ballistic
