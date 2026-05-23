#include "DebugMessenger.h"

bool DebugMessenger::Create(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, bool enableValidation)
{
    if (!enableValidation) return true;
    VK_CHECK_HANDLE(instance, VkInstance);
    
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
        LOG_ERROR("Debug utils extension not present");
        return false;
    }

    if (func(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan debug messenger");
        return false;
    }

    LOG_DEBUG("Debug Messenger created");
    return true;
}

void DebugMessenger::Destroy()
{
    if (messenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(instanceHandle, messenger, nullptr);
        messenger = VK_NULL_HANDLE;
        instanceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Debug Messenger destroyed");
    }
}

VKAPI_ATTR VkBool32 DebugMessenger::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) LOG_ERROR("[VULKAN] %s", data->pMessage);
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) LOG_WARN("[VULKAN] %s", data->pMessage);
    else LOG_DEBUG("[VULKAN] %s", data->pMessage);
    return VK_FALSE;
}
