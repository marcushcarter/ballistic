#pragma once
#include "bepch.h"

namespace ballistic
{
    struct WindowSettings {
        std::string title = "Ballistic Engine";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool customTitleBar = false;
        bool resizable = true;

        static WindowSettings LoadSettingsFromProject() {
            WindowSettings settings;
            
            settings.customTitleBar = false;

            return settings;
        }
    };

    struct WindowState {
        int pub = 0;
    };
    
} // namespace ballistic
