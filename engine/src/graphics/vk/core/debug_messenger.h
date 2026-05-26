#pragma once
#include "pch.h"

struct DebugMessenger
{
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback = nullptr;
    void* pUserData = nullptr;
    
    bool Create(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, VkDebugUtilsMessageTypeFlagsEXT messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, bool enableValidation = true);
    void Destroy();
    
    VkDebugUtilsMessengerEXT Get() const { return messenger; }
    
private:
    VkInstance instanceHandle = VK_NULL_HANDLE;
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);
};
