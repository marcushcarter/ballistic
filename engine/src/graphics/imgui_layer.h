#pragma once
#include "pch.h"
#include "vk/vk.h"

struct Renderer;

struct ImGuiLayer
{
    DescriptorPool descriptorPool;

    bool Create(Renderer& renderer, GLFWwindow* window);
    void Destroy();

    void NewFrame();
    void Render();
    void RecordDraw(VkCommandBuffer cmd);
};