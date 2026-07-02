#pragma once
#include <core/log/error.h>
#include <vulkan/vulkan.h>
#include <windows.h>
#include <string>

namespace ballistic::drivers {

struct ImGuiDriverCreateInfo
{
    HWND hwnd = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queue_family = 0;
    VkQueue queue = VK_NULL_HANDLE;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    uint32_t image_count = 2;
    const char* ini_path = nullptr;
    bool enable_docking = false;
};

struct ImGuiDriver
{
    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    std::string ini_path_storage;

    Error create(const ImGuiDriverCreateInfo& p_create_info);
    void destroy();

    void new_frame();
    void render();
    void record_commands(VkCommandBuffer p_cmd);
};

}