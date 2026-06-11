#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <functional>

namespace Ballistic
{
struct Window
{
    GLFWwindow* glfwWindow = nullptr;
    uint32_t width = 1280, height = 800;
    bool fullscreen = false;

    uint32_t windowedWidth = 1280, windowedHeight = 800;
    int windowedX = 0, windowedY = 0;

    std::function<void(uint32_t, uint32_t)> onFramebufferResize;

    bool Create(const char* title, uint32_t width, uint32_t height);
    void Destroy();

    bool ShouldClose();
    void PollEvents();

    void Show() const { glfwShowWindow(glfwWindow); }
    void Hide() const { glfwHideWindow(glfwWindow); }

    void SetFullscreen(bool enabled);
    void ToggleFullscreen() { SetFullscreen(!fullscreen); }
    void SetTitle(const char* title);
    void SetTitlebarColor(float r, float g, float b);

    GLFWwindow* GetNative() { return glfwWindow; }

private:
    inline static Window* instance;
};
}