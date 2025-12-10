#pragma once

#include "lrpch.h"

namespace Ballistic {

	struct WindowProps {
		std::string title;
		int width;
		int height;
		bool VSync;

		WindowProps(const std::string title = std::string("BallisticEngine"),
			int width = 1280,
			int height = 720,
			bool VSync = false)
			: width(width), height(height), VSync(VSync), title(title) {}
	};

	class Window {
	public:
		Window(const WindowProps& windowProps);
		~Window();

		void onUpdate();
		bool shouldClose();

		static std::shared_ptr<Window> createWindow(const WindowProps windowProps = {});

	private:
		GLFWwindow* m_NativeWindow;
		
	};
}