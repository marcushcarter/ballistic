#pragma once
#include "pch.h"

struct Window
{
    GLFWwindow* glfwWindow = nullptr;
    uint32_t width = 1280, height = 800;
    bool fullscreen = false;

    uint32_t windowedWidth = 1280, windowedHeight = 800;
    int windowedX = 0, windowedY = 0;

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
    
    bool SetIcon(const char* path);
    void DefaultIcon();

private:
    inline static Window* instance;
};