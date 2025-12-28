#pragma once
#include "bepch.h"

namespace ballistic
{
    struct WindowSettings {
        std::string title = "Ballistic Engine";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool vsync = false;
        bool customTitleBar = false;
        bool resizable = true;

        static WindowSettings LoadSettingsFromProject() {
            WindowSettings settings;
            settings.customTitleBar = false;
            return settings;
        }
    };

    struct WindowState {
		int width = 1280;
		int height = 720;
		float dpiScale = 1.0f;
		bool fullscreen = false;
		bool minimized = false;
		bool maximized = false;
		bool focused = true;

		int windowedX = 0;
		int windowedY = 0;
		int windowedW = 1280;
		int windowedH = 720;
    };
    
} // namespace ballistic
