#pragma once
#include "bepch.h"

namespace Ballistic {

	struct WindowProps;
	struct WindowState;
	class WindowAPI;
	
	class IWindow {
	public:
		virtual ~IWindow() = default;
		
		using WindowPtr = std::shared_ptr<IWindow>;

		virtual bool ShouldClose() const = 0;
		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;
		virtual void OnUpdate() = 0;

		virtual WindowProps GetProps() const = 0;
		virtual WindowState GetState() const = 0;

		virtual void ToggleFullscreen(bool fullscreen) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetTitle(const std::string& title) = 0;
		virtual void SetSize(int width, int height) = 0;
		virtual void SetPosition(int x, int y) = 0;
		virtual void Focus() = 0;
		virtual void Minimize() = 0;
		virtual void Maximize(bool enabled) = 0;

		virtual void SetResizeCallback(std::function<void(int,int)> callback) = 0;
		virtual void SetFocusCallback(std::function<void(bool)> callback) = 0;
		virtual void SetCloseCallback(std::function<void()> callback) = 0;

		virtual void* GetNativeWindow() const = 0;

		static WindowPtr Create(const WindowProps& props);
	};

}