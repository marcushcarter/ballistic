#pragma once
#include "bepch.h"

namespace Ballistic {

	struct WindowProps {
		std::string title = "BallisticEngine";
		int width = 1280;
		int height = 720;
		bool vsync = false;

		WindowProps() = default;
		WindowProps(const std::string& title, int width, int height, bool vsync = false)
			: title(title), width(width), height(height), vsync(vsync) {}
	};

	class WindowAPI {
	public:
		enum class API { None = 0, GLFW = 1 };
		static API GetAPI() { return s_api; }
		static void SetAPI(API api) { s_api = api; }
	private:
		static API s_api;
	};

	struct WindowState {
		int width = 1280;
		int height = 720;
		float dpiScale = 1.0f;
		bool fullscreen = false;

		int windowedX = 0;
		int windowedY = 0;
		int windowedW = 1280;
		int windowedH = 720;

		bool focused = true;
		bool minimized = false;
		bool maximized = false;
	};
}