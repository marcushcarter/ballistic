#pragma once
#include <core/error/error.h>
#include <windows.h>
#include <string>

namespace ballistic::drivers {

struct WindowDriverWin32
{
    HWND hwnd = nullptr;
    bool close_requested = false;
    uint32_t pending_width = 0;
    uint32_t pending_height = 0;
    bool resize_requested = false;

    Error create(const std::wstring& p_title, int p_width, int p_height);
    void destroy();

    void poll_events();
    bool should_close() const { return close_requested; }
    void request_close() { close_requested = true; }

    Error set_icon(HICON p_icon);
    Error set_titlebar_color(COLORREF p_color);

    static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
};

}