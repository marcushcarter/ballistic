#pragma once
#include "pch.h"

struct Window
{
    GLFWwindow* glfwWindow = nullptr;
    // uint32_t width = 1280, height = 800;

    bool Create(const std::string& title, uint32_t width, uint32_t height);
    void Destroy();

    bool ShouldClose();
    void PollEvents();

    void Show() const { glfwShowWindow(glfwWindow); }
    void Hide() const { glfwHideWindow(glfwWindow); }

    // void SetTitle(const std::string& title);
    
    // bool SetIcon(const std::string& path);
    void DefaultIcon();
};