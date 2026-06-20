#pragma once
#include <core/error/error.h>
#include <vulkan/vulkan.h>
#include <windows.h>

namespace ballistic::drivers {

struct ImGuiDriverCreateInfo
{
    HWND hwnd = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queue_family = 0;
    VkQueue queue = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    uint32_t image_count = 2;
};

struct ImGuiDriver
{
    VkDescriptorPool descriptor_pool;

    Error create(const ImGuiDriverCreateInfo& p_info);
    void destroy();

    void new_frame();
    void render();
    void record_commands(VkCommandBuffer p_cmd);
};

}