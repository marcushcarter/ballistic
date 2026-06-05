#pragma once
#include <vk/descriptor/descriptor_pool.h>

struct Renderer;
struct GLFWwindow;

struct ImGuiLayer
{
    DescriptorPool descriptorPool;

    bool Create(Renderer& renderer, GLFWwindow* window);
    void Destroy();

    void NewFrame();
    void Render();
    void RecordDraw(VkCommandBuffer cmd);
};