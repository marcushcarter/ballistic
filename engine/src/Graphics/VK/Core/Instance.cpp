#include "Instance.h"

bool Instance::Create(const char* name, uint32_t major, uint32_t minor, uint32_t patch, const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& requiredLayers, bool enableValidation)
{
    Destroy();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    appInfo.pEngineName = APP_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> layers = requiredLayers;

    if (enableValidation)
        layers.push_back("VK_LAYER_KHRONOS_validation");

    if (!CheckLayerSupport(layers)) {
        LOG_ERROR("Instance validation layers unavailable");
        layers.clear();
    }

    if (!CheckExtensionSupport(requiredExtensions)) {
        LOG_ERROR("Required extensions missing");
        return false;
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }

    LOG_DEBUG("Instance Created");
    return true;
}

void Instance::Destroy()
{
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
        LOG_DEBUG("Instance Destroyed");
    }
}

bool Instance::CheckLayerSupport(const std::vector<const char*>& layers)
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());

    for (const char* layerName : layers) {
        bool found = false;
        for (const auto& layer : available) {
            if (strcmp(layer.layerName, layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}

bool Instance::CheckExtensionSupport(const std::vector<const char*>& extensions)
{
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

    for (const char* ext : extensions) {
        bool found = false;
        for (const auto& e : available) {
            if (strcmp(e.extensionName, ext) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}
