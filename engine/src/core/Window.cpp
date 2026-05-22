#include "Window.h"

#define IDR_WIN_ICON 101

bool Window::Create(const char* title, uint32_t w, uint32_t h)
{
    if (!glfwInit()) {
        LOG_FATAL("Failed to initialize GLFW");
        return false;
    }

    this->width = w;
    this->height = h;

    glfwWindowHint(GLFW_CLIENT_API,  GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    // glfwWindowHint(GLFW_VISIBLE, desc.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!glfwWindow) {
        LOG_FATAL("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(glfwWindow, this);
    // glfwSetFramebufferSizeCallback(glfwWindow, FramebufferSizeCallback);
    // glfwSetKeyCallback(glfwWindow, KeyCallback);
    // glfwSetCursorPosCallback(glfwWindow, MouseCallback);
    // glfwSetJoystickCallback(JoystickCallback);

    instance = this;

    LOG_DEBUG("Window created: %dx%d", w, h);
    return true;
}

void Window::Destroy()
{    
    if (glfwWindow) {
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
        glfwWindow = nullptr;
        LOG_DEBUG("Window destroyed");
    }
}

bool Window::ShouldClose() { return glfwWindowShouldClose(glfwWindow); }

void Window::PollEvents() { glfwPollEvents(); }

void Window::SetFullscreen(bool enabled)
{
    if (fullscreen == enabled) return;
    fullscreen = enabled;

    if (enabled) {
        glfwGetWindowSize(glfwWindow, (int*)&windowedWidth, (int*)&windowedHeight);
        glfwGetWindowPos(glfwWindow, &windowedX, &windowedY);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        width  = mode->width;
        height = mode->height;
    } else {
        glfwSetWindowMonitor(glfwWindow, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
        width  = windowedWidth;
        height = windowedHeight;
    }
}

void Window::SetTitle(const char* title) { if (glfwWindow) glfwSetWindowTitle(glfwWindow, title); }

void Window::SetTitlebarColor(float r, float g, float b)
{
    HWND hwnd = glfwGetWin32Window(glfwWindow);
    COLORREF color = RGB((int)(r * 255), (int)(g * 255), (int)(b * 255));
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));
}

bool Window::SetIcon(const char* path)
{
    if (!glfwWindow) return false;

    int w, h, channels;
    stbi_uc* data = stbi_load(path, &w, &h, &channels, 4);
    if (!data) {
        LOG_ERROR("Window failed to load icon: %s", path);
        return false;
    }

    GLFWimage icon;
    icon.width = w;
    icon.height = h;
    icon.pixels = data;
    glfwSetWindowIcon(glfwWindow, 1, &icon);

    stbi_image_free(data);
    return true;
}

void Window::DefaultIcon()
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(IDR_WIN_ICON), RT_RCDATA);
    if (!res) { LOG_ERROR("Window failed to find default icon"); return; }

    HGLOBAL mem = LoadResource(nullptr, res);
    if (!mem) { LOG_ERROR("Window failed to load default icon"); return; }

    void* data = LockResource(mem);
    if (!data) { LOG_ERROR("Window failed to lock default icon"); return; }

    DWORD size = SizeofResource(nullptr, res);

    int iconW, iconH, channels;
    unsigned char* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &iconW, &iconH, &channels, 4);
    if (!pixels) { LOG_ERROR("Window failed to load default icon from memory"); return; }

    GLFWimage icon{ iconW, iconH, pixels };
    glfwSetWindowIcon(glfwWindow, 1, &icon);
    stbi_image_free(pixels);
}
