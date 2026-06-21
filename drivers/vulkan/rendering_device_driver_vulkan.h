#pragma once

#include <drivers/vulkan/rendering_context_driver_vulkan.h>

#include <shaderc/shaderc.hpp>
#include <vk_mem_alloc.h>

#include <drivers/vulkan/ballistic_vulkan.h>

namespace ballistic::drivers {

struct RenderingDeviceDriverVulkan
{
	struct Queue {
		VkQueue queue = VK_NULL_HANDLE;
	};

    struct SubgroupCapabilities {
        uint32_t size = 0;
        VkShaderStageFlags supported_stages = 0;
        VkSubgroupFeatureFlags supported_operations = 0;
    };

    VkDevice device = VK_NULL_HANDLE;
    RenderingContextDriverVulkan* context_driver = nullptr;
    uint32_t device_index = 0;
    DriverDevice context_device;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t frame_count = 1;
	VkPhysicalDeviceProperties physical_device_properties = {};
	VkPhysicalDeviceFeatures physical_device_features = {};
	VkPhysicalDeviceFeatures requested_device_features = {};
    std::unordered_map<std::string, bool> requested_device_extensions;
    std::unordered_set<std::string> enabled_device_extension_names;
    std::vector<std::vector<Queue>> queue_families;
    SubgroupCapabilities subgroup_capabilities;

	void _register_requested_device_extension(const std::string& p_extension_name, bool p_required);
    Error _initialize_device_extensions();
    void _get_device_properties();
    Error _check_device_features();
    Error _check_device_capabilities();
    Error _add_queue_create_info(std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info);
    Error _initialize_device(const std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info);
    Error _initialize_allocator();
    Error _initialize_pipeline_cache();

    void _check_subgroup_capabilities();

    Error initialize(uint32_t p_device_index, uint32_t p_frame_count);

    VmaAllocator allocator = nullptr;
};

}