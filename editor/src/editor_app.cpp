#include "editor_app.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <stb_image.h>

#define IDR_WIN_ICON 101

static void set_window_icon(GLFWwindow* window)
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
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(pixels);
}

int editor_main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Ballistic Editor", nullptr, nullptr);

    set_window_icon(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}