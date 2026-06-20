#pragma once
#include <windows.h>
#include <string>

namespace ballistic::drivers {

struct WindowDriverWin32
{
    HWND hwnd = nullptr;
    bool close_requested = false;

    void create(const std::wstring& p_title, int p_width, int p_height);
    void destroy();

    void poll_events();
    bool should_close() const { return close_requested; }
    void request_close() { close_requested = true; }

    void set_icon(HICON p_icon);
    void set_titlebar_color(COLORREF p_color);

    static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
};

}