#include "window.h"

#define IDR_WIN_ICON 101

bool Window::Create(const std::string& title, uint32_t width, uint32_t height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    return glfwWindow != nullptr;
}

void Window::Destroy()
{
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    glfwWindow = nullptr;
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(glfwWindow);
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::DefaultIcon()
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(IDR_WIN_ICON), RT_RCDATA);
    if (!res) { OutputDebugStringA("set_window_icon: FindResource failed\n"); return; }

    HGLOBAL mem = LoadResource(nullptr, res);
    if (!mem) { OutputDebugStringA("set_window_icon: LoadResource failed\n"); return; }

    void* data = LockResource(mem);
    if (!data) { OutputDebugStringA("set_window_icon: LockResource failed\n"); return; }

    DWORD size = SizeofResource(nullptr, res);

    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &width, &height, &channels, 4);
    if (!pixels) { OutputDebugStringA("set_window_icon: stbi_load_from_memory failed\n"); return; }

    GLFWimage icon{ width, height, pixels };
    glfwSetWindowIcon(glfwWindow, 1, &icon);
    stbi_image_free(pixels);
}
