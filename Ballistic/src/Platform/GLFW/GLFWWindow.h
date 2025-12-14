#pragma once

#include "bepch.h"
#include "Core/IWindow.h"

namespace Ballistic {
	
	class GLFWWindow : public IWindow {
	public:
		GLFWWindow(const WindowProps& windowProps);
		~GLFWWindow();

		void onUpdate() override;
		bool shouldClose() const override;

		void toggleFullscreen(bool fullscreen) override;
		bool isFullscreen() const override;

		void* get() const override { return m_NativeWindow; }
		WindowProps getProps() const override { return m_Props; }

		static std::shared_ptr<IWindow> Create(const WindowProps& windowProps = {});

	private:
		GLFWwindow* m_NativeWindow;
		WindowProps m_Props;

		int m_WindowedX = 0, m_WindowedY = 0, m_WindowedW = 1280, m_WindowedH = 720;
	};

}