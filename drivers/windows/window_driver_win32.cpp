#include <drivers/windows/window_driver_win32.h>
#include <backends/imgui_impl_win32.h>
#include <dwmapi.h>
#include <windowsx.h>

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
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

Error WindowDriverWin32::window_create(const std::string& p_title, int p_width, int p_height, bool p_custom_titlebar)
{
    using enum Error;

    window.custom_titlebar = p_custom_titlebar;
    
    std::wstring title = utf8_to_wstring(p_title);

    window.hwnd = CreateWindowExW(
        0, BALLISTIC_WINDOW_CLASS, title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, p_width, p_height,
        nullptr, nullptr, GetModuleHandleW(nullptr), this
    );

    BALLISTIC_ERR_FAIL_COND_V_MSG(!window.hwnd, Failed, "Couldn't create Win32 window.");

    window.width = static_cast<uint32_t>(p_width);
    window.height = static_cast<uint32_t>(p_height);

    return Ok;
}

void WindowDriverWin32::window_bind()
{
    if (!window.hwnd) return;
    SetWindowLongPtrW(window.hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&window));

    if (window.custom_titlebar) {
        DWORD corner = DWMWCP_ROUND;
        DwmSetWindowAttribute(window.hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        SetWindowPos(window.hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    ShowWindow(window.hwnd, SW_SHOW);
}

void WindowDriverWin32::window_free()
{
    if (window.hwnd) {
        DestroyWindow(window.hwnd);
        window.hwnd = nullptr;
    }
}

bool WindowDriverWin32::window_should_close()
{
    return window.close_requested;
}

void WindowDriverWin32::window_request_close()
{
    window.close_requested = true;
}

Error WindowDriverWin32::window_set_icon(HICON p_icon)
{
    using enum Error;
    BALLISTIC_ERR_FAIL_COND_V(!p_icon, Failed);
    BALLISTIC_ERR_FAIL_COND_V(!window.hwnd, Failed);
    SendMessageW(window.hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(p_icon));
    SendMessageW(window.hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(p_icon));
    return Ok;
}

Error WindowDriverWin32::window_set_title(std::string_view p_title)
{
    using enum Error;
    if (!window.hwnd) return Failed;
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, p_title.data(), (int)p_title.size(), nullptr, 0);
    if (wide_len <= 0) return Failed;
    std::wstring wide(wide_len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, p_title.data(), (int)p_title.size(), wide.data(), wide_len);
    return SetWindowTextW(window.hwnd, wide.c_str()) ? Ok : Failed;
}

Error WindowDriverWin32::window_set_titlebar_color(COLORREF p_color)
{
    using enum Error;
    BALLISTIC_ERR_FAIL_COND_V(!window.hwnd, Failed);
    HRESULT result = DwmSetWindowAttribute(window.hwnd, DWMWA_CAPTION_COLOR, &p_color, sizeof(p_color));
    BALLISTIC_ERR_FAIL_COND_V_MSG(FAILED(result), Failed, "Failed to set Win32 window titlebar color - DWMWA_CAPTION_COLOR requires Windows 11 (build 22000+).");
    return Ok;
}

void WindowDriverWin32::window_minimize()
{
    if (window.hwnd) ShowWindow(window.hwnd, SW_MINIMIZE);
}

void WindowDriverWin32::window_toggle_maximize()
{
    if (!window.hwnd) return;
    ShowWindow(window.hwnd, IsZoomed(window.hwnd) ? SW_RESTORE : SW_MAXIMIZE);
}

bool WindowDriverWin32::window_is_maximized()
{
    return window.hwnd && IsZoomed(window.hwnd);
}

void WindowDriverWin32::window_set_custom_titlebar(bool p_enabled)
{
    if (!window.hwnd || window.custom_titlebar == p_enabled) return;
    window.custom_titlebar = p_enabled;
    DWORD corner = p_enabled ? DWMWCP_ROUND : 0;
    DwmSetWindowAttribute(window.hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
    SetWindowPos(window.hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    if (!p_enabled) {
        window.titlebar_height = 0;
        window.titlebar_interactive_rects.clear();
    }
}

void WindowDriverWin32::window_titlebar_reset(int height)
{
    window.titlebar_height = height;
    window.titlebar_interactive_rects.clear();
}

void WindowDriverWin32::window_titlebar_add_rect(long l, long t, long r, long b)
{
    window.titlebar_interactive_rects.push_back(RECT{ l, t, r, b });
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
        case WM_NCCALCSIZE: {
            if (!p_wparam || !window || !window->custom_titlebar) break;
            NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(p_lparam);
            RECT& rect = params->rgrc[0];
            if (IsZoomed(p_hwnd)) {
                int fx = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                int fy = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                rect.left += fx;
                rect.right -= fx;
                rect.top += fy;
                rect.bottom -= fy;
            }
            return 0;
        }

        case WM_NCHITTEST: {
            if (!window || !window->custom_titlebar) break;
            POINT cursor = { GET_X_LPARAM(p_lparam), GET_Y_LPARAM(p_lparam) };
            RECT client;
            ScreenToClient(p_hwnd, &cursor);
            GetClientRect(p_hwnd, &client);

            bool over_widget = false;
            if (cursor.y < window->titlebar_height) {
                for (const RECT& r : window->titlebar_interactive_rects) {
                    if (cursor.x >= r.left && cursor.x < r.right && cursor.y >= r.top  && cursor.y < r.bottom) {
                        over_widget = true;
                        break;
                    }
                }
            }
                
            const int border = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            if (!IsZoomed(p_hwnd)) {
                bool l = cursor.x < border;
                bool r = cursor.x >= client.right - border;
                bool t = cursor.y < border;
                bool b = cursor.y >= client.bottom - border;
                if (t && l) return HTTOPLEFT;
                if (t && r) return HTTOPRIGHT;
                if (b && l) return HTBOTTOMLEFT;
                if (b && r) return HTBOTTOMRIGHT;
                if (l) return HTLEFT;
                if (r) return HTRIGHT;
                if (b) return HTBOTTOM;
                if (t && !over_widget) return HTTOP;
            }
            if (over_widget) return HTCLIENT;
            if (cursor.y < window->titlebar_height) return HTCAPTION;
            return HTCLIENT;
        }

        case WM_CLOSE: {
            if (window) window->close_requested = true;
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE: {
            if (window) {
                window->width = LOWORD(p_lparam);
                window->height = HIWORD(p_lparam);
            }
            return 0;
        }
    }

    return DefWindowProcW(p_hwnd, p_msg, p_wparam, p_lparam);
}

}