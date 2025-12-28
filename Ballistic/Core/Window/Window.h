#pragma once
#include "bepch.h"
#include "WindowInfo.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ballistic
{
    class Window
    {
    public:
        Window() = default;
        ~Window() { Shutdown(); }

        bool Init(const WindowSettings& windowSettings = {});
        bool ShouldClose() const;
        void Update(float deltaTime);
        void Shutdown();

        const WindowSettings& GetSettings() const { return m_settings; }
        const WindowState& GetState() const { return m_state; }

        GLFWwindow* GetNativeWindow() const { return m_nativeWindow; }
        
    private:
        WindowSettings m_settings;
        WindowState m_state;
        GLFWwindow* m_nativeWindow;
    };

} // namespace ballistic
