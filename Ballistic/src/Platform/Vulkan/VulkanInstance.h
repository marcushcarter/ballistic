#pragma once

#include "bepch.h"

namespace Ballistic {

	class VulkanInstance {
	public:
		struct Config {
	        bool enableValidation = true;
	        bool useGlfwExtensions = true;
	        const char* appName = "Ballistic Engine";
	        uint32_t appVersion = VK_MAKE_VERSION(1,0,0);
	        const char* engineName = "Ballistic Engine";
	        uint32_t engineVersion = VK_MAKE_VERSION(1,0,0);
	        uint32_t apiVersion = VK_API_VERSION_1_3;

	        std::vector<const char*> extraExtensions;
	    };

	    VulkanInstance(const Config& cfg = {});
	    ~VulkanInstance();

	    VkInstance get() const { return instance; }

    private:
    	VkInstance instance = VK_NULL_HANDLE;
	    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	    Config config;

	    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT /*severity*/,
        VkDebugUtilsMessageTypeFlagsEXT /*type*/,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*) {
	        std::cerr << "Vulkan: " << pCallbackData->pMessage << std::endl;
	        return VK_FALSE;
	    }

	};
}