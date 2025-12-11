#pragma once

#include "bepch.h"

namespace Ballistic {

	struct WindowProps {
		std::string title;
		int width;
		int height;
		bool VSync;

		WindowProps(const std::string& title = "BallisticEngine", int width = 1280, int height = 720, bool VSync = false)
			: title(title), width(width), height(height), VSync(VSync) {}
	};
	
	class GLFWWindow {
	public:
		GLFWWindow(const WindowProps& windowProps);
		~GLFWWindow();

		void onUpdate();
		bool shouldClose() const;

		static std::shared_ptr<GLFWWindow> CreateWindow(const WindowProps& windowProps = {});

	private:
		GLFWwindow* m_NativeWindow;
	};

}