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
        void Update();
        void Shutdown();

        void* GetNativeWindow() const { return m_nativeWindow; }
        
    private:
        WindowSettings m_settings;
        WindowState m_state;
        GLFWwindow* m_nativeWindow;
        
        bool m_dragging = false;
        double m_dragOffsetX = 0.0;
        double m_dragOffsetY = 0.0;
    };

} // namespace ballistic
