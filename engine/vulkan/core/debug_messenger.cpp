#include <core/debug_messenger.h>
#include <iostream>
#include <windows.h>

namespace Ballistic::Vulkan
{
bool DebugMessenger::Create(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, bool enableValidation)
{
    if (!enableValidation) return true;
    
    Destroy();
    instanceHandle = instance;

    if (!pfnUserCallback) {
        pfnUserCallback = DebugCallback;
        pUserData = this;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = messageSeverity;
    createInfo.messageType = messageType;
    createInfo.pfnUserCallback = pfnUserCallback;
    createInfo.pUserData = pUserData;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (!func) {
        // LOG_ERROR("Debug utils extension not present");
        return false;
    }

    if (func(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan debug messenger");
        return false;
    }

    // LOG_DEBUG("Debug Messenger created");
    return true;
}

void DebugMessenger::Destroy()
{
    if (messenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(instanceHandle, messenger, nullptr);
        messenger = VK_NULL_HANDLE;
        // LOG_DEBUG("Debug Messenger destroyed");
    }
}

VKAPI_ATTR VkBool32 DebugMessenger::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, 12);

    const char* level = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN" : "INFO";
    std::cerr << "\n[VULKAN][" << level << "] " << data->pMessage << "\n";

    
    // if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    //     SetConsoleTextAttribute(console, 12);
    //     std::cerr << "\n[VULKAN][ERROR] " << data->pMessage << "\n";

    // } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    //     SetConsoleTextAttribute(console, 12);
    //     std::cerr << "\n[VULKAN][WARN] " << data->pMessage << "\n";

    // } else {
    //     SetConsoleTextAttribute(console, 7);
    //     std::cerr << "\n[VULKAN][INFO] " << data->pMessage << "\n";

    // }
    
    SetConsoleTextAttribute(console, 7);
    return VK_FALSE;
}
}