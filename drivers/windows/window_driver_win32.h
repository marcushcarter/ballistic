#pragma once
#include <core/log/error.h>
#include <windows.h>
#include <string>
#include <vector>

namespace ballistic::drivers {

struct WindowDriverWin32
{
    /***************/
    /**** SETUP ****/
    /***************/

    Error initialize();
    void shutdown();

    /****************/
    /**** WINDOW ****/
    /****************/

    struct Window {
        HWND hwnd = nullptr;
        bool close_requested = false;
        uint32_t width = 0;
        uint32_t height = 0;

        bool custom_titlebar = false;
        int  titlebar_height = 0;
        std::vector<RECT> titlebar_interactive_rects;
    };

    Window window;

    static void poll_events();

    Error window_create(const std::string& p_title, int p_width, int p_height, bool p_custom_titlebar = false);
    void window_bind();
    void window_free();

    bool window_should_close();
    void window_request_close();

    Error window_set_icon(HICON p_icon);
    Error window_set_title(std::string_view p_title);
    Error window_set_titlebar_color(COLORREF p_color);

    void window_minimize();
    void window_toggle_maximize();
    bool window_is_maximized();
    
    void window_set_custom_titlebar(bool p_enabled);
    void window_titlebar_reset(int height);
    void window_titlebar_add_rect(long left, long top, long right, long bottom);

    static bool system_accent_color(float& r_r, float& r_g, float& r_b);

    static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
};

}