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

	class IWindow {
	public:
		virtual ~IWindow() = default;

		virtual void onUpdate() = 0;
		
		virtual bool shouldClose() = 0;

		static std::shared_ptr<IWindow> createWindow(const WindowProps windowProps = {});
	};
}