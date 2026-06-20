#include <drivers/windows/window_driver_win32.h>
#include <core/error/error_macros.h>
#include <backends/imgui_impl_win32.h>
#include <dwmapi.h>

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, ImGuiIO& io);

namespace ballistic::drivers {    

Error WindowDriverWin32::create(const std::wstring& p_title, int p_width, int p_height)
{
    using enum Error;
    
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

    BALLISTIC_ERR_FAIL_COND_V(!hwnd, Failed);

    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd, SW_SHOW);

    return Ok;
}

void WindowDriverWin32::destroy()
{
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void WindowDriverWin32::poll_events()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

Error WindowDriverWin32::set_icon(HICON p_icon)
{
    using enum Error;

    BALLISTIC_ERR_FAIL_COND_V(!p_icon, Failed);

    SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(p_icon));
    SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(p_icon));

    return Ok;
}

Error WindowDriverWin32::set_titlebar_color(COLORREF p_color)
{    
    using enum Error;

    HRESULT result = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &p_color, sizeof(p_color));

    BALLISTIC_ERR_FAIL_COND_V_MSG(FAILED(result), Failed,
        "DwmSetWindowAttribute failed — DWMWA_CAPTION_COLOR requires Windows 11 (build 22000+).");

    return Ok;
}

LRESULT CALLBACK WindowDriverWin32::wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(p_hwnd, p_msg, p_wparam, p_lparam))
        return true;

    auto* self = reinterpret_cast<WindowDriverWin32*>(GetWindowLongPtrW(p_hwnd, GWLP_USERDATA));

    switch (p_msg) {
        case WM_CLOSE:
            if (self) self->close_requested = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (self) {
                uint32_t new_width = LOWORD(p_lparam);
                uint32_t new_height = HIWORD(p_lparam);

                if (new_width > 0 && new_height > 0) {
                    self->pending_width = new_width;
                    self->pending_height = new_height;
                    self->resize_requested = true;
                }
            }
            return 0;
    }

    return DefWindowProcW(p_hwnd, p_msg, p_wparam, p_lparam);
}

}