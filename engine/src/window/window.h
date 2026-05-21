#pragma once
#include "pch.h"

struct Window
{
    GLFWwindow* window = nullptr;

    bool Create(const char* title, int width, int height);
    void Destroy();

    bool ShouldClose();
    void PollEvents();

    void DefaultIcon();
};