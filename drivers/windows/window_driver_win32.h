#pragma once
#include <core/log/error.h>
#include <windows.h>
#include <string>

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
    };

    static void poll_events();

    Window window_create(const std::string& p_title, int p_width, int p_height);
    void window_bind(Window& r_window);
    void window_free(Window& r_window);

    bool window_should_close(const Window& r_window);
    void window_request_close(Window& r_window);

    Error window_set_icon(Window& r_window, HICON p_icon);
    Error window_set_titlebar_color(Window& r_window, COLORREF p_color);

    static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
};

}