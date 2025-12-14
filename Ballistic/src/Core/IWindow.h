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

	class WindowAPI {
	public:
		enum class API {
			None = 0,
			GLFW = 1,
		};

		static API GetAPI() { return s_API; }
		static void SetAPI(API api) { s_API = api; }

	private:
		static API s_API;
	};
	
	class IWindow {
	public:
		virtual ~IWindow() = default;

		virtual void onUpdate() = 0;
		virtual bool shouldClose() const = 0;

		virtual void toggleFullscreen(bool fullscreen) = 0;
		virtual bool isFullscreen() const = 0;

		virtual void* get() const = 0;
		virtual WindowProps getProps() const = 0;
	};

}