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

    static void poll_events();

    Window window_create(const std::string& p_title, int p_width, int p_height, bool p_custom_titlebar = false);
    void window_bind(Window& r_window);
    void window_free(Window& r_window);

    bool window_should_close(const Window& r_window);
    void window_request_close(Window& r_window);

    Error window_set_icon(Window& r_window, HICON p_icon);
    Error window_set_title(Window& r_window, std::string_view p_title);
    Error window_set_titlebar_color(Window& r_window, COLORREF p_color);

    void window_minimize(Window& r_window);
    void window_toggle_maximize(Window& r_window);
    bool window_is_maximized(const Window& r_window);

    void window_titlebar_reset(Window& r_window, int height);
    void window_titlebar_add_rect(Window& r_window, long left, long top, long right, long bottom);

    static bool system_accent_color(float& r_r, float& r_g, float& r_b);

    static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
};

}