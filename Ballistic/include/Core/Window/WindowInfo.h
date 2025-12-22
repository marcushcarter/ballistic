#pragma once
#include "bepch.h"

namespace Ballistic {

	struct WindowProps {
		std::string title = "Ballistic Engine";
		int width = 1280;
		int height = 720;
		bool vsync = false;
		bool imgui = true;
		bool viewports = false;
		bool dockspace = false;

		WindowProps() = default;

		WindowProps& SetTitle(const std::string& t) { title = t; return *this; }
		WindowProps& SetSize(int w, int h) { width = w; height = h; return *this; }
		WindowProps& SetVSync(bool e = true) { vsync = e; return *this; }
		WindowProps& SetImGui(bool e = true, bool viewportsFlag = false, bool dockspaceFlag = false) {
			imgui = e;
			viewports = viewportsFlag;
			dockspace = dockspaceFlag;
			return *this;
		}
	};

	struct WindowPresets {
		static WindowProps Runtime() {
			return WindowProps()
				.SetTitle("Runtime")
				.SetSize(1280, 720)
				.SetVSync(true)
				.SetImGui(false);			
		}
		
		static WindowProps Editor() {
			return WindowProps()
				.SetTitle("Editor")
				.SetSize(1600, 900)
				.SetVSync(false)
				.SetImGui(true, true, true);
		}
		
		// static WindowProps& FromConfig(const Config& cgf) {
		// }
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