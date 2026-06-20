#include <drivers/windows/window_driver_windows.h>
#include <backends/imgui_impl_win32.h>
#include <dwmapi.h>

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, ImGuiIO& io);

namespace ballistic {    

void WindowDriverWindows::create(const std::wstring& p_title, int p_width, int p_height)
{
    WNDCLASSW wc{};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"BallisticWindowClass";
    RegisterClassW(&wc);

    hwnd = CreateWindowExW(
        0, L"BallisticWindowClass", p_title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, p_width, p_height,
        nullptr, nullptr, wc.hInstance, this
    );

    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd, SW_SHOW);
}

void WindowDriverWindows::destroy()
{
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void WindowDriverWindows::poll_events()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void WindowDriverWindows::set_icon(HICON p_icon)
{
    if (p_icon) {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(p_icon));
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(p_icon));
    }
}

void WindowDriverWindows::set_titlebar_color(COLORREF p_color)
{
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &p_color, sizeof(p_color));
}

LRESULT CALLBACK WindowDriverWindows::wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(p_hwnd, p_msg, p_wparam, p_lparam))
        return true;

    auto* self = reinterpret_cast<WindowDriverWindows*>(GetWindowLongPtrW(p_hwnd, GWLP_USERDATA));

    switch (p_msg) {
        case WM_CLOSE:
            if (self) self->close_requested = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(p_hwnd, p_msg, p_wparam, p_lparam);
}

}