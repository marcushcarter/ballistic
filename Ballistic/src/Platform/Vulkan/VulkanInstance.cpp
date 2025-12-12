#include "VulkanInstance.h"

namespace Ballistic {

	VulkanInstance::VulkanInstance(const Config& cfg) {

		uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        if (layerCount > 0)
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        if (config.enableValidation) {
            bool found = false;
            for (auto& l : availableLayers)
                if (strcmp(l.layerName, validationLayer) == 0) { found = true; break; }
            if (!found) throw std::runtime_error("Validation layer not found");
        }

        // ===== Required extensions =====

        std::vector<const char*> extensions;
        if (config.useGlfwExtensions) {
            uint32_t glfwExtCount = 0;
            const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
            for (uint32_t i = 0; i < glfwExtCount; ++i) extensions.push_back(glfwExt[i]);
        }
        if (config.enableValidation) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // ===== Application info =====
        
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = config.appName;
        appInfo.applicationVersion = config.appVersion;
        appInfo.pEngineName = config.engineName;
        appInfo.engineVersion = config.engineVersion;
        appInfo.apiVersion = config.apiVersion;

        // ===== Instance create info =====

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (config.enableValidation) {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayer;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan instance");

        // ===== Debug messenger =====

        if (config.enableValidation) {
            VkDebugUtilsMessengerCreateInfoEXT dbgInfo{};
            dbgInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            dbgInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            dbgInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            dbgInfo.pfnUserCallback = debugCallback;

            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

            if (func) func(instance, &dbgInfo, nullptr, &debugMessenger);
        }

	}

	VulkanInstance::~VulkanInstance() {
		if (config.enableValidation && debugMessenger) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func) func(instance, debugMessenger, nullptr);
        }
        if (instance) vkDestroyInstance(instance, nullptr);
	}

	
}