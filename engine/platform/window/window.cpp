#include <window/window.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <dwmapi.h>
#include <iostream>

namespace Ballistic
{
bool Window::Create(const char* title, uint32_t w, uint32_t h)
{
    if (!glfwInit()) {
        // LOG_FATAL("Failed to initialize GLFW");
        // std::cout << "BAD BAD BAD\n";
        return false;
    }

    this->width = w;
    this->height = h;

    glfwWindowHint(GLFW_CLIENT_API,  GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!glfwWindow) {
        // LOG_FATAL("Failed to create GLFW window");
        // std::cout << "BAD BAD BAD\n";
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(glfwWindow, this);
    // glfwSetFramebufferSizeCallback(glfwWindow, FramebufferSizeCallback);
    // glfwSetKeyCallback(glfwWindow, KeyCallback);
    // glfwSetCursorPosCallback(glfwWindow, MouseCallback);
    // glfwSetJoystickCallback(JoystickCallback);

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* win, int w, int h) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
        self->width  = (uint32_t)w;
        self->height = (uint32_t)h;
        if (self->onFramebufferResize) self->onFramebufferResize((uint32_t)w, (uint32_t)h);
    });

    instance = this;

    // LOG_DEBUG("Window created: %dx%d", w, h);
    return true;
}

void Window::Destroy()
{    
    if (glfwWindow) {
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
        glfwWindow = nullptr;
        // LOG_DEBUG("Window destroyed");
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
}