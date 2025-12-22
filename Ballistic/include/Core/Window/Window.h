#pragma once
#include "bepch.h"
#include "Core/Window/WindowInfo.h"

namespace Ballistic {

	class ImGuiSystem;
	
	class Window {
	public:
		Window(const WindowProps& props);
		~Window();
        
		static void InitGLFW();
		static void PollEvents();

		bool ShouldClose() const;
		void SwapBuffers();
		void OnUpdate();

		void BeginFrame();
		void EndFrame();

		WindowProps GetProps() const { return m_props; }
		WindowState GetState() const { return m_state; }

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

		GLFWwindow* GetNativeWindow() const { return m_window; }

		bool HasImGuiContext() const { return m_props.imgui; }

	private:
		GLFWwindow* m_window;
		WindowProps m_props;
		WindowState m_state;

		std::unique_ptr<ImGuiSystem> m_imguiSystem;

		std::function<void(int,int)> m_resizeCallback;
		std::function<void(bool)> m_focusCallback;
		std::function<void()> m_closeCallback;
	};

}