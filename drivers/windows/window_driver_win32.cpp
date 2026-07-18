#include <drivers/windows/window_driver_win32.h>
#include <core/log/error_macros.h>
#include <backends/imgui_impl_win32.h>
#include <dwmapi.h>

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, ImGuiIO& io);

namespace ballistic::drivers {
    
static const wchar_t* BALLISTIC_WINDOW_CLASS = L"BallisticWindowClass";

static std::wstring utf8_to_wstring(const std::string& str)
{
    if (str.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
    return result;
}

Error WindowDriverWin32::initialize()
{
    using enum Error;

    WNDCLASSW wc{};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = BALLISTIC_WINDOW_CLASS;
    RegisterClassW(&wc);

    return Ok;
}

void WindowDriverWin32::shutdown()
{
    UnregisterClassW(BALLISTIC_WINDOW_CLASS, GetModuleHandleW(nullptr));
}

void WindowDriverWin32::poll_events()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

WindowDriverWin32::Window WindowDriverWin32::window_create(const std::string& p_title, int p_width, int p_height)
{
    Window window;
    
    std::wstring title = utf8_to_wstring(p_title);

    window.hwnd = CreateWindowExW(
        0, BALLISTIC_WINDOW_CLASS, title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, p_width, p_height,
        nullptr, nullptr, GetModuleHandleW(nullptr), this
    );

    BALLISTIC_ERR_FAIL_COND_V_MSG(!window.hwnd, {}, "Couldn't create Win32 window.");

    window.width = static_cast<uint32_t>(p_width);
    window.height = static_cast<uint32_t>(p_height);

    return window;

}

void WindowDriverWin32::window_bind(Window& r_window)
{
    if (!r_window.hwnd) return;
    SetWindowLongPtrW(r_window.hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&r_window));
    ShowWindow(r_window.hwnd, SW_SHOW);
}

void WindowDriverWin32::window_free(Window& r_window)
{
    if (r_window.hwnd) {
        DestroyWindow(r_window.hwnd);
        r_window.hwnd = nullptr;
    }
}

bool WindowDriverWin32::window_should_close(const Window& r_window)
{
    return r_window.close_requested;
}

void WindowDriverWin32::window_request_close(Window& r_window)
{
    r_window.close_requested = true;
}

Error WindowDriverWin32::window_set_icon(Window& r_window, HICON p_icon)
{
    using enum Error;
    BALLISTIC_ERR_FAIL_COND_V(!p_icon, Failed);
    BALLISTIC_ERR_FAIL_COND_V(!r_window.hwnd, Failed);
    SendMessageW(r_window.hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(p_icon));
    SendMessageW(r_window.hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(p_icon));
    return Ok;
}

Error WindowDriverWin32::window_set_title(Window& r_window, std::string_view p_title)
{
    using enum Error;
    if (!r_window.hwnd) return Failed;
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, p_title.data(), (int)p_title.size(), nullptr, 0);
    if (wide_len <= 0) return Failed;
    std::wstring wide(wide_len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, p_title.data(), (int)p_title.size(), wide.data(), wide_len);
    return SetWindowTextW(r_window.hwnd, wide.c_str()) ? Ok : Failed;
}

Error WindowDriverWin32::window_set_titlebar_color(Window& r_window, COLORREF p_color)
{
    using enum Error;
    BALLISTIC_ERR_FAIL_COND_V(!r_window.hwnd, Failed);
    HRESULT result = DwmSetWindowAttribute(r_window.hwnd, DWMWA_CAPTION_COLOR, &p_color, sizeof(p_color));
    BALLISTIC_ERR_FAIL_COND_V_MSG(FAILED(result), Failed, "Failed to set Win32 window titlebar color - DWMWA_CAPTION_COLOR requires Windows 11 (build 22000+).");
    return Ok;
}

bool WindowDriverWin32::system_accent_color(float& r_r, float& r_g, float& r_b)
{
    DWORD argb = 0; BOOL opaque = FALSE;
    if (FAILED(DwmGetColorizationColor(&argb, &opaque))) return false;
    r_r = ((argb >> 16) & 0xFF) / 255.0f;
    r_g = ((argb >>  8) & 0xFF) / 255.0f;
    r_b = ( argb        & 0xFF) / 255.0f;
    return true;
}

LRESULT CALLBACK WindowDriverWin32::wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(p_hwnd, p_msg, p_wparam, p_lparam))
        return true;

    auto* window = reinterpret_cast<Window*>(GetWindowLongPtrW(p_hwnd, GWLP_USERDATA));

    switch (p_msg) {
        case WM_CLOSE:
            if (window) window->close_requested = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (window) {
                window->width = LOWORD(p_lparam);
                window->height = HIWORD(p_lparam);
            }
            return 0;
    }

    return DefWindowProcW(p_hwnd, p_msg, p_wparam, p_lparam);
}

}