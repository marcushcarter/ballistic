#pragma once
#include "bepch.h"
#include "Core/Windows/IWindow.h"
#include "Core/Windows/WindowInfo.h"

namespace Ballistic {
	
	class GLFWWindow : public IWindow {
	public:
		GLFWWindow(const WindowProps& windowProps);
		~GLFWWindow();

		bool ShouldClose() const override;
		void PollEvents() override;
		void SwapBuffers() override;

		void OnUpdate() override;

		WindowProps GetProps() const override { return m_props; }
		WindowState GetState() const override { return m_state; }

		void ToggleFullscreen(bool fullscreen) override;
		void SetVSync(bool enabled) override;
		void SetTitle(const std::string& title) override;
		void SetSize(int width, int height) override;
		void SetPosition(int x, int y) override;
		void Focus() override;
		void Minimize() override;
		void Maximize(bool enabled) override;

		void SetResizeCallback(std::function<void(int,int)> callback) override { m_resizeCallback = callback; }
		void SetFocusCallback(std::function<void(bool)> callback) override { m_focusCallback = callback; }
		void SetCloseCallback(std::function<void()> callback) override { m_closeCallback = callback; }

		void* GetNativeWindow() const override { return m_window; }

	private:
		GLFWwindow* m_window;
		WindowProps m_props;
		WindowState m_state;

		std::function<void(int,int)> m_resizeCallback;
		std::function<void(bool)> m_focusCallback;
		std::function<void()> m_closeCallback;

		void initGLFW();
		void setupCallbacks();
	};

}