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
        Window();
        ~Window() { Shutdown(); }

        bool Init(const WindowSettings& windowSettings = {});
        bool ShouldClose() const;
        void Present();
        void Shutdown();

        const WindowSettings& GetSettings() const { return m_settings; }
        const WindowState& GetState() const { return m_state; }

        bool SetIcon(const std::filesystem::path& path);
        bool SetIcon(const void* pixels, int width, int height, int channels = 4);

        void ToggleFullscreen(bool fullscreen);
		void SetVSync(bool enabled);
		void SetTitle(const std::string& title);
		void SetSize(int width, int height);
		void SetPosition(int x, int y);
		void Focus();
		void Minimize();
		void Maximize(bool enabled);

        void SetResizeCallback(std::function<void(int,int)> callback) { m_resizeCallback = callback; }
		void SetFocusCallback(std::function<void(bool)> callback) { m_focusCallback = callback; }
		void SetCloseCallback(std::function<void()> callback) { m_closeCallback = callback; }

        GLFWwindow* GetNativeWindow() const { return m_nativeWindow; }
        
    private:
        GLFWwindow* m_nativeWindow = nullptr;
        WindowSettings m_settings;
        WindowState m_state;

        std::function<void(int,int)> m_resizeCallback;
		std::function<void(bool)> m_focusCallback;
		std::function<void()> m_closeCallback;
    };

} // namespace ballistic
