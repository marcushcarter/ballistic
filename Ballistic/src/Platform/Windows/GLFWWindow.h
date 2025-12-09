#pragma once

#include "lrpch.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

	class GLFWWindowIMPL {
	public:
		GLFWWindowIMPL(const WindowProps& windowProps);
		~GLFWWindowIMPL();

		void onUpdate();
		bool shouldClose();

		static std::shared_ptr<GLFWWindowIMPL> createWindow(const WindowProps windowProps = {});

	private:
		GLFWwindow* m_NativeWindow;
		
	};
}