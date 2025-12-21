#pragma once
#include "bepch.h"

namespace Ballistic {

	struct WindowProps {
		std::string title = "Ballistic Engine";
		int width = 1280;
		int height = 720;
		bool vsync = false;

		WindowProps() = default;
		WindowProps(const std::string& title, int width, int height, bool vsync = false)
			: title(title), width(width), height(height), vsync(vsync) {}
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