#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/log/error_macros.h>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <algorithm>
#include <mutex>

namespace ballistic::drivers {

/***************/
/**** SETUP ****/
/***************/

void DeviceDriverVulkan::_register_requested_device_extension(const std::string& p_extension_name, bool p_required) {
    requested_device_extensions[p_extension_name] = p_required;
}

Error DeviceDriverVulkan::_initialize_device_extensions()
{
    using enum Error;

    requested_device_extensions.clear();
    enabled_device_extension_names.clear();

    _register_requested_device_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
    _register_requested_device_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, true);
    _register_requested_device_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, true);

    _register_requested_device_extension(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME, false);

#ifdef BALLISTIC_EDITOR
    _register_requested_device_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_EXT_DEVICE_FAULT_EXTENSION_NAME, false);
    _register_requested_device_extension(VK_NV_RAY_TRACING_VALIDATION_EXTENSION_NAME, false);

#endif

    // if (features.variable_rate_shading) {
    //     _register_requested_device_extension(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, false);
    // }

    // if (features.subgroup_size_control) {
    //     _register_requested_device_extension(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, false);
    // }

    // if (features.ray_tracing) {
    //     _register_requested_device_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, true);
    //     _register_requested_device_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, true);
    //     _register_requested_device_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, true);
    // }

    uint32_t device_extension_count = 0;
    VkResult err = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "vkEnumerateDeviceExtensionProperties (count query) failed.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(device_extension_count == 0, Failed, "Couldn't find any Vulkan device extensions. Do you have a compatible Vulkan installable client driver (ICD) installed?");

	std::vector<VkExtensionProperties> device_extensions;
    device_extensions.resize(device_extension_count);
    err = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data());
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan device extension properties.");
    
    for (uint32_t i = 0; i < device_extension_count; i++) {
        std::string extension_name(device_extensions[i].extensionName);
        if (requested_device_extensions.contains(extension_name)) {
            enabled_device_extension_names.insert(extension_name);
        }
    }

    for (const auto& [name, is_required] : requested_device_extensions) {
        if (!enabled_device_extension_names.contains(name)) {
            BALLISTIC_ERR_FAIL_COND_V_MSG(is_required, Failed, ("Required Vulkan device extension " + name + " was not found.").c_str());
        }
    }

    return Ok;
}

void DeviceDriverVulkan::_get_device_properties()
{
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

    std::string device_type = vk::to_string(vk::PhysicalDeviceType(physical_device_properties.deviceType));

    log_write("Vulkan %u.%u.%u - Driver %u.%u.%u - Using Device: %s (%s)",
        VK_API_VERSION_MAJOR(physical_device_properties.apiVersion),
        VK_API_VERSION_MINOR(physical_device_properties.apiVersion),
        VK_API_VERSION_PATCH(physical_device_properties.apiVersion),
        VK_API_VERSION_MAJOR(physical_device_properties.driverVersion),
        VK_API_VERSION_MINOR(physical_device_properties.driverVersion),
        VK_API_VERSION_PATCH(physical_device_properties.driverVersion),
        physical_device_properties.deviceName,
        device_type.c_str());
}

Error DeviceDriverVulkan::_check_device_features()
{
    using enum Error;

    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    if (!physical_device_features.imageCubeArray || !physical_device_features.independentBlend) {
        std::string error_string = "Your GPU does not support required Vulkan features:\n";
        if (!physical_device_features.imageCubeArray) error_string += "- No support for image cube arrays.\n";
        if (!physical_device_features.independentBlend) error_string += "- No support for independentBlend.\n";
        error_string += "This is usually a hardware limitation; updating drivers won't help.";
        BALLISTIC_ERR_FAIL_COND_V_MSG(true, Failed, error_string.c_str());
    }

#define VK_DEVICEFEATURE_ENABLE_IF(x) \
    if (physical_device_features.x) { \
        requested_device_features.x = physical_device_features.x; \
    } else \
        ((void)0)

    requested_device_features = {};
    VK_DEVICEFEATURE_ENABLE_IF(fullDrawIndexUint32);
    VK_DEVICEFEATURE_ENABLE_IF(imageCubeArray);
    VK_DEVICEFEATURE_ENABLE_IF(independentBlend);
    VK_DEVICEFEATURE_ENABLE_IF(geometryShader);
    VK_DEVICEFEATURE_ENABLE_IF(tessellationShader);
    VK_DEVICEFEATURE_ENABLE_IF(sampleRateShading);
    VK_DEVICEFEATURE_ENABLE_IF(dualSrcBlend);
    VK_DEVICEFEATURE_ENABLE_IF(logicOp);
    VK_DEVICEFEATURE_ENABLE_IF(multiDrawIndirect);
    VK_DEVICEFEATURE_ENABLE_IF(drawIndirectFirstInstance);
    VK_DEVICEFEATURE_ENABLE_IF(depthClamp);
    VK_DEVICEFEATURE_ENABLE_IF(depthBiasClamp);
    VK_DEVICEFEATURE_ENABLE_IF(fillModeNonSolid);
    VK_DEVICEFEATURE_ENABLE_IF(depthBounds);
    VK_DEVICEFEATURE_ENABLE_IF(wideLines);
    VK_DEVICEFEATURE_ENABLE_IF(largePoints);
    VK_DEVICEFEATURE_ENABLE_IF(samplerAnisotropy);
    VK_DEVICEFEATURE_ENABLE_IF(textureCompressionBC);
    VK_DEVICEFEATURE_ENABLE_IF(vertexPipelineStoresAndAtomics);
    VK_DEVICEFEATURE_ENABLE_IF(fragmentStoresAndAtomics);
    VK_DEVICEFEATURE_ENABLE_IF(shaderClipDistance);
    VK_DEVICEFEATURE_ENABLE_IF(shaderCullDistance);
    VK_DEVICEFEATURE_ENABLE_IF(shaderInt64);
    VK_DEVICEFEATURE_ENABLE_IF(shaderInt16);

#undef VK_DEVICEFEATURE_ENABLE_IF

    return Ok;
}

void DeviceDriverVulkan::_check_subgroup_capabilities()
{
    VkPhysicalDeviceSubgroupProperties subgroup_properties{};
    subgroup_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

    VkPhysicalDeviceProperties2 properties2{};
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties2.pNext = &subgroup_properties;

    vkGetPhysicalDeviceProperties2(physical_device, &properties2);

    subgroup_capabilities.size = subgroup_properties.subgroupSize;
    subgroup_capabilities.supported_stages = subgroup_properties.supportedStages;
    subgroup_capabilities.supported_operations = subgroup_properties.supportedOperations;
}

Error DeviceDriverVulkan::_check_device_capabilities()
{
    using enum Error;

    _check_subgroup_capabilities();

    return Ok;
}

Error DeviceDriverVulkan::_add_queue_create_info(std::vector<VkDeviceQueueCreateInfo> &r_queue_create_info)
{
    using enum Error;

    static const float queue_priority = 1.0f;

    std::vector<uint32_t> distinct_families = {
        context_driver->graphics_queue_family,
        context_driver->present_queue_family,
        context_driver->transfer_queue_family,
        context_driver->compute_queue_family
    };

    std::sort(distinct_families.begin(), distinct_families.end());
    distinct_families.erase(std::unique(distinct_families.begin(), distinct_families.end()), distinct_families.end());

    queue_families.resize(context_driver->queue_family_get_count(device_index));

    for (uint32_t family_index : distinct_families) {
        VkDeviceQueueCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = family_index;
        create_info.queueCount = 1;
        create_info.pQueuePriorities = &queue_priority;
        r_queue_create_info.push_back(create_info);
        queue_families[family_index].resize(1);
    }

    return Ok;
}

Error DeviceDriverVulkan::_initialize_device(const std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info)
{
    using enum Error;

    std::vector<const char*> enabled_extension_names;
    enabled_extension_names.reserve(enabled_device_extension_names.size());
    for (const std::string& extension_name : enabled_device_extension_names) {
        enabled_extension_names.push_back(extension_name.c_str());
    }

    VkPhysicalDeviceFeatures2 supported_features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    VkPhysicalDeviceVulkan12Features supported_1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    supported_features2.pNext = &supported_1_2;
    vkGetPhysicalDeviceFeatures2(physical_device, &supported_features2);

    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.descriptorIndexing, Failed, "GPU lacks descriptorIndexing, required for bindless rendering.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.runtimeDescriptorArray, Failed, "GPU lacks runtimeDescriptorArray, required for bindless rendering.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.descriptorBindingPartiallyBound, Failed, "GPU lacks descriptorBindingPartiallyBound.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.descriptorBindingSampledImageUpdateAfterBind, Failed, "GPU lacks descriptorBindingSampledImageUpdateAfterBind.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.descriptorBindingStorageImageUpdateAfterBind, Failed, "GPU lacks descriptorBindingStorageImageUpdateAfterBind.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.descriptorBindingVariableDescriptorCount, Failed, "GPU lacks descriptorBindingVariableDescriptorCount.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.shaderSampledImageArrayNonUniformIndexing, Failed, "GPU lacks shaderSampledImageArrayNonUniformIndexing.");
    BALLISTIC_ERR_FAIL_COND_V_MSG(!supported_1_2.shaderStorageImageArrayNonUniformIndexing, Failed, "GPU lacks shaderStorageImageArrayNonUniformIndexing.");

    void* create_info_next = nullptr;

    VkPhysicalDeviceVulkan13Features features_1_3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features_1_3.dynamicRendering = VK_TRUE;
    features_1_3.synchronization2 = VK_TRUE;
    features_1_3.pNext = create_info_next;
    create_info_next = &features_1_3;

    VkPhysicalDeviceVulkan12Features features_1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features_1_2.descriptorIndexing = VK_TRUE;
    features_1_2.runtimeDescriptorArray = VK_TRUE;
    features_1_2.bufferDeviceAddress = VK_TRUE;
    features_1_2.descriptorBindingPartiallyBound = VK_TRUE;
    features_1_2.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features_1_2.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    features_1_2.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features_1_2.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features_1_2.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    features_1_2.pNext = create_info_next;
    create_info_next = &features_1_2;

    // VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shader_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR };
	// shader_features.shaderFloat16 = shader_capabilities.shader_float16_is_supported;
	// shader_features.shaderInt8 = shader_capabilities.shader_int8_is_supported;
	// shader_features.pNext = create_info_next;
	// create_info_next = &shader_features;

// 	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address_features = {};
// 	if (buffer_device_address_support) {
// 		buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
// 		buffer_device_address_features.pNext = create_info_next;
// 		buffer_device_address_features.bufferDeviceAddress = buffer_device_address_support;
// 		create_info_next = &buffer_device_address_features;
// 	}

// 	VkPhysicalDeviceVulkanMemoryModelFeaturesKHR vulkan_memory_model_features = {};
// 	if (vulkan_memory_model_support && vulkan_memory_model_device_scope_support) {
// 		vulkan_memory_model_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR;
// 		vulkan_memory_model_features.pNext = create_info_next;
// 		vulkan_memory_model_features.vulkanMemoryModel = vulkan_memory_model_support;
// 		vulkan_memory_model_features.vulkanMemoryModelDeviceScope = vulkan_memory_model_device_scope_support;
// 		create_info_next = &vulkan_memory_model_features;
// 	}

// 	VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = {};
// 	if (fsr_capabilities.pipeline_supported || fsr_capabilities.primitive_supported || fsr_capabilities.attachment_supported) {
// 		fsr_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
// 		fsr_features.pNext = create_info_next;
// 		fsr_features.pipelineFragmentShadingRate = fsr_capabilities.pipeline_supported;
// 		fsr_features.primitiveFragmentShadingRate = fsr_capabilities.primitive_supported;
// 		fsr_features.attachmentFragmentShadingRate = fsr_capabilities.attachment_supported;
// 		create_info_next = &fsr_features;
// 	}

// 	VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = {};
// 	if (fdm_capabilities.attachment_supported || fdm_capabilities.dynamic_attachment_supported || fdm_capabilities.non_subsampled_images_supported) {
// 		fdm_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
// 		fdm_features.pNext = create_info_next;
// 		fdm_features.fragmentDensityMap = fdm_capabilities.attachment_supported;
// 		fdm_features.fragmentDensityMapDynamic = fdm_capabilities.dynamic_attachment_supported;
// 		fdm_features.fragmentDensityMapNonSubsampledImages = fdm_capabilities.non_subsampled_images_supported;
// 		create_info_next = &fdm_features;
// 	}

// 	VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM fdm_offset_features = {};
// 	if (fdm_capabilities.offset_supported) {
// 		fdm_offset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM;
// 		fdm_offset_features.pNext = create_info_next;
// 		fdm_offset_features.fragmentDensityMapOffset = VK_TRUE;
// 		create_info_next = &fdm_offset_features;
// 	}

// 	VkPhysicalDevicePipelineCreationCacheControlFeatures pipeline_cache_control_features = {};
// 	if (pipeline_cache_control_support) {
// 		pipeline_cache_control_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
// 		pipeline_cache_control_features.pNext = create_info_next;
// 		pipeline_cache_control_features.pipelineCreationCacheControl = pipeline_cache_control_support;
// 		create_info_next = &pipeline_cache_control_features;
// 	}

// 	VkPhysicalDeviceFaultFeaturesEXT device_fault_features = {};
// 	if (device_fault_support) {
// 		device_fault_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT;
// 		device_fault_features.pNext = create_info_next;
// 		create_info_next = &device_fault_features;
// 	}

// #if defined(VK_TRACK_DEVICE_MEMORY)
// 	VkDeviceDeviceMemoryReportCreateInfoEXT memory_report_info = {};
// 	if (device_memory_report_support) {
// 		memory_report_info.sType = VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT;
// 		memory_report_info.pfnUserCallback = ContextDriverVulkan::memory_report_callback;
// 		memory_report_info.pNext = create_info_next;
// 		memory_report_info.flags = 0;
// 		memory_report_info.pUserData = this;

// 		create_info_next = &memory_report_info;
// 	}
// #endif

// 	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
// 	if (acceleration_structure_capabilities.acceleration_structure_support) {
// 		acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
// 		acceleration_structure_features.pNext = create_info_next;
// 		acceleration_structure_features.accelerationStructure = acceleration_structure_capabilities.acceleration_structure_support;
// 		create_info_next = &acceleration_structure_features;
// 	}

// 	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline_features = {};
// 	if (raytracing_capabilities.raytracing_pipeline_support) {
// 		raytracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
// 		raytracing_pipeline_features.pNext = create_info_next;
// 		raytracing_pipeline_features.rayTracingPipeline = raytracing_capabilities.raytracing_pipeline_support;
// 		create_info_next = &raytracing_pipeline_features;
// 	}

// 	VkPhysicalDeviceRayTracingValidationFeaturesNV raytracing_validation_features = {};
// 	if (raytracing_capabilities.validation) {
// 		raytracing_validation_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV;
// 		raytracing_validation_features.pNext = create_info_next;
// 		raytracing_validation_features.rayTracingValidation = raytracing_capabilities.validation;
// 		create_info_next = &raytracing_validation_features;
// 	}

// 	VkPhysicalDeviceVulkan11Features vulkan_1_1_features = {};
// 	VkPhysicalDevice16BitStorageFeaturesKHR storage_features = {};
// 	VkPhysicalDeviceMultiviewFeatures multiview_features = {};
// 	const bool enable_1_2_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_2;
// 	if (enable_1_2_features) {
// 		// In Vulkan 1.2 and newer we use a newer struct to enable various features.
// 		vulkan_1_1_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
// 		vulkan_1_1_features.pNext = create_info_next;
// 		vulkan_1_1_features.storageBuffer16BitAccess = storage_buffer_capabilities.storage_buffer_16_bit_access_is_supported;
// 		vulkan_1_1_features.uniformAndStorageBuffer16BitAccess = storage_buffer_capabilities.uniform_and_storage_buffer_16_bit_access_is_supported;
// 		vulkan_1_1_features.storagePushConstant16 = storage_buffer_capabilities.storage_push_constant_16_is_supported;
// 		vulkan_1_1_features.storageInputOutput16 = storage_buffer_capabilities.storage_input_output_16;
// 		vulkan_1_1_features.multiview = multiview_capabilities.is_supported;
// 		vulkan_1_1_features.multiviewGeometryShader = multiview_capabilities.geometry_shader_is_supported;
// 		vulkan_1_1_features.multiviewTessellationShader = multiview_capabilities.tessellation_shader_is_supported;
// 		vulkan_1_1_features.variablePointersStorageBuffer = 0;
// 		vulkan_1_1_features.variablePointers = 0;
// 		vulkan_1_1_features.protectedMemory = 0;
// 		vulkan_1_1_features.samplerYcbcrConversion = 0;
// 		vulkan_1_1_features.shaderDrawParameters = 0;
// 		create_info_next = &vulkan_1_1_features;
// 	} else {
// 		// On Vulkan 1.0 and 1.1 we use our older structs to initialize these features.
// 		storage_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
// 		storage_features.pNext = create_info_next;
// 		storage_features.storageBuffer16BitAccess = storage_buffer_capabilities.storage_buffer_16_bit_access_is_supported;
// 		storage_features.uniformAndStorageBuffer16BitAccess = storage_buffer_capabilities.uniform_and_storage_buffer_16_bit_access_is_supported;
// 		storage_features.storagePushConstant16 = storage_buffer_capabilities.storage_push_constant_16_is_supported;
// 		storage_features.storageInputOutput16 = storage_buffer_capabilities.storage_input_output_16;
// 		create_info_next = &storage_features;

// 		const bool enable_1_1_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_1;
// 		if (enable_1_1_features) {
// 			multiview_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
// 			multiview_features.pNext = create_info_next;
// 			multiview_features.multiview = multiview_capabilities.is_supported;
// 			multiview_features.multiviewGeometryShader = multiview_capabilities.geometry_shader_is_supported;
// 			multiview_features.multiviewTessellationShader = multiview_capabilities.tessellation_shader_is_supported;
// 			create_info_next = &multiview_features;
// 		}
// 	}

    VkDeviceCreateInfo device_ci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_ci.pNext = create_info_next;
    device_ci.queueCreateInfoCount = static_cast<uint32_t>(p_queue_create_info.size());
    device_ci.pQueueCreateInfos = p_queue_create_info.data();
    device_ci.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
    device_ci.ppEnabledExtensionNames = enabled_extension_names.data();
    device_ci.pEnabledFeatures = &requested_device_features;
   
    VkResult err = vkCreateDevice(physical_device, &device_ci, nullptr, &device);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed,
        "vkCreateDevice failed. Do you have a compatible Vulkan driver installed?");

	for (uint32_t i = 0; i < queue_families.size(); i++) {
		for (uint32_t j = 0; j < queue_families[i].size(); j++) {
			vkGetDeviceQueue(device, i, j, &queue_families[i][j].queue);
		}
	}

    // Set PFN_Functions

    return Ok;
}

Error DeviceDriverVulkan::_initialize_allocator()
{
    using enum Error;

    VmaAllocatorCreateInfo allocator_ci{};
    allocator_ci.physicalDevice = physical_device;
    allocator_ci.device = device;
    allocator_ci.instance = context_driver->instance_get();
    const bool use_1_3_features = physical_device_properties.apiVersion >= VK_API_VERSION_1_3;
    if (use_1_3_features) {
        allocator_ci.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
    }
	if (/*buffer_device_address_support*/ true) {
		allocator_ci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}

    VkResult err = vmaCreateAllocator(&allocator_ci, &allocator);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed,
        "Couldn't create Vulkan memory allocator.");

    return Ok;
}

Error DeviceDriverVulkan::_initialize_pipeline_cache()
{
    using enum Error;

    return Ok;
}

Error DeviceDriverVulkan::initialize(ContextDriverVulkan& r_context_driver, uint32_t p_device_index, uint32_t p_frame_count)
{
    using enum Error;
    Error err;

    context_driver = &r_context_driver;

    device_index = p_device_index;
    driver_device = context_driver->device_get(device_index);
    physical_device = context_driver->physical_device_get(device_index);
	frame_count = p_frame_count;
    
    err = _initialize_device_extensions();
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    _get_device_properties();
    
    err = _check_device_features();
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    err = _check_device_capabilities();
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    std::vector<VkDeviceQueueCreateInfo> queue_create_info;
    err = _add_queue_create_info(queue_create_info);
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    err = _initialize_device(queue_create_info);
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    err = _initialize_allocator();
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    err = _initialize_pipeline_cache();
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    err = swapchain_create(&context_driver->surface);
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = swapchain_resize(frame_count);
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = bindless_heap_create(16384, 4096, 256);
	BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

void DeviceDriverVulkan::shutdown()
{
    device_wait_idle();

    bindless_heap_free();

    swapchain_free();

    // while (small_allocs_pools.size()) {
    //     HashMap<uint32_t, VmaPool>::Iterator E = small_allocs_pools.begin();
    //     vmaDestroyPool(allocator, E->value);
    //     small_allocs_pools.remove(E);
    // }

    vmaDestroyAllocator(allocator);

    // for (KeyValue<int, DescriptorSetPools> &pool_map : linear_descriptor_set_pools) {
    //     for (KeyValue<DescriptorSetPoolKey, HashMap<VkDescriptorPool, uint32_t>> pools : pool_map.value) {
    //         for (KeyValue<VkDescriptorPool, uint32_t> descriptor_pool : pools.value) {
    //             vkDestroyDescriptorPool(vk_device, descriptor_pool.key, VKC::get_allocation_callbacks(VK_OBJECT_TYPE_DESCRIPTOR_POOL));
    //         }
    //     }
    // }

    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
}

Error DeviceDriverVulkan::device_wait_idle()
{    
    using enum Error;
    VkResult err = vkDeviceWaitIdle(device);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't wait idle for Vulkan device.");
    return Ok;
}

/****************/
/**** FENCES ****/
/****************/

VkFence DeviceDriverVulkan::fence_create(bool p_signaled)
{
    VkFenceCreateInfo fence_ci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fence_ci.flags = p_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    
    VkFence fence;
    VkResult err = vkCreateFence(device, &fence_ci, nullptr, &fence);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, VK_NULL_HANDLE, "Couldn't create Vulkan fence.");
    
    return fence;
}

void DeviceDriverVulkan::fence_free(VkFence& r_fence)
{
    if (r_fence) {
        vkDestroyFence(device, r_fence, nullptr);
        r_fence = VK_NULL_HANDLE;
    }
}

Error DeviceDriverVulkan::fence_wait(VkFence p_fence, uint64_t p_timeout)
{
    using enum Error;
    VkResult err = vkWaitForFences(device, 1, &p_fence, VK_TRUE, p_timeout);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't wait for Vulkan fence.");
    return Ok;
}

Error DeviceDriverVulkan::fence_reset(VkFence p_fence)
{
    using enum Error;
    VkResult err = vkResetFences(device, 1, &p_fence);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't reset Vulkan fence.");
    return Ok;
}

/********************/
/**** SEMAPHORES ****/
/********************/

VkSemaphore DeviceDriverVulkan::semaphore_create()
{
    VkSemaphoreCreateInfo semaphore_ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    semaphore_ci.pNext = nullptr;
    semaphore_ci.flags = 0;
    
    VkSemaphore semaphore;
    VkResult err = vkCreateSemaphore(device, &semaphore_ci, nullptr, &semaphore);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, VK_NULL_HANDLE, "Couldn't create Vulkan semaphore.");
    
    return semaphore;
}

void DeviceDriverVulkan::semaphore_free(VkSemaphore& r_semaphore)
{
    if (r_semaphore) {
        vkDestroySemaphore(device, r_semaphore, nullptr);
        r_semaphore = VK_NULL_HANDLE;
    }
}

/******************/
/**** COMMANDS ****/
/******************/

// ----- POOL -----

DeviceDriverVulkan::CommandPool DeviceDriverVulkan::command_pool_create(uint32_t p_queue_family_index, VkCommandBufferLevel p_buffer_level)
{
    VkCommandPoolCreateInfo cmd_pool_ci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmd_pool_ci.queueFamilyIndex = p_queue_family_index;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    
    CommandPool cmd_pool;
    cmd_pool.buffer_level = p_buffer_level;
    VkResult err = vkCreateCommandPool(device, &cmd_pool_ci, nullptr, &cmd_pool.command_pool);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't create Vulkan command pool.");

    return cmd_pool;
}

void DeviceDriverVulkan::command_pool_free(CommandPool& r_cmd_pool)
{
    if (r_cmd_pool.command_pool) {
        vkDestroyCommandPool(device, r_cmd_pool.command_pool, nullptr);
        r_cmd_pool.command_pool = VK_NULL_HANDLE;
    }
}

Error DeviceDriverVulkan::command_pool_reset(CommandPool& r_cmd_pool)
{
    using enum Error;
    VkResult err = vkResetCommandPool(device, r_cmd_pool.command_pool, 0);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't reset Vulkan command pool.");
    return Ok;
}

// ----- BUFFER -----

VkCommandBuffer DeviceDriverVulkan::command_buffer_create(CommandPool& p_cmd_pool)
{
    VkCommandBufferAllocateInfo cmd_buffer_ci{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmd_buffer_ci.commandPool = p_cmd_pool.command_pool;
    cmd_buffer_ci.level = p_cmd_pool.buffer_level;
    cmd_buffer_ci.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vkAllocateCommandBuffers(device, &cmd_buffer_ci, &cmd_buffer);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, VK_NULL_HANDLE, "Couldn't create Vulkan command buffer.");
    
    return cmd_buffer;
}

Error DeviceDriverVulkan::command_buffer_begin(VkCommandBuffer p_cmd_buffer, VkCommandBufferUsageFlags p_flags)
{
    using enum Error;
    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = p_flags;
    VkResult err = vkBeginCommandBuffer(p_cmd_buffer, &begin_info);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't begin Vulkan command buffer.");
    return Ok;
}

Error DeviceDriverVulkan::command_buffer_end(VkCommandBuffer p_cmd_buffer)
{
    using enum Error;
    VkResult err = vkEndCommandBuffer(p_cmd_buffer);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't end Vulkan command buffer.");
    return Ok;
}

/****************/
/**** IMAGES ****/
/****************/

DeviceDriverVulkan::Image DeviceDriverVulkan::image_create(const ImageCreateInfo& p_create_info, VkExtent3D p_extent)
{
    using enum Error;
    
    Image image;
    image.extent = p_extent;
    image.format = p_create_info.format;
    image.aspect= p_create_info.aspect;
    image.mip_levels = p_create_info.mip_levels;
    image.layers = p_create_info.layers;
    
    VkImageCreateInfo image_ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    image_ci.flags = 0;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = p_create_info.format;
    image_ci.extent = p_extent;
    image_ci.mipLevels = p_create_info.mip_levels;
    image_ci.arrayLayers = p_create_info.layers;
    image_ci.samples = p_create_info.samples;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = p_create_info.usage;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VkResult err = vkCreateImage(device, &image_ci, nullptr, &image.image);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't create Vulkan image.");

    vkGetImageMemoryRequirements(device, image.image, &image.mem_req);
    set_object_name(VK_OBJECT_TYPE_IMAGE, (uint64_t)image.image, p_create_info.name);
    return image;
}

DeviceDriverVulkan::Image DeviceDriverVulkan::image_create_dedicated(const ImageCreateInfo& p_create_info, VkExtent3D p_extent)
{
    using enum Error;

    Image image = image_create(p_create_info, p_extent);
    BALLISTIC_ERR_FAIL_COND_V(image.image == VK_NULL_HANDLE, {});

    VmaAllocationCreateInfo alloc_ci{};
    alloc_ci.usage = VMA_MEMORY_USAGE_UNKNOWN;
    alloc_ci.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VmaAllocation allocation = nullptr;
    VkResult err = vmaAllocateMemoryForImage(allocator, image.image, &alloc_ci, &allocation, nullptr);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't allocate image memory.");

    Error e = image_bind(image, allocation);
    BALLISTIC_ERR_FAIL_COND_V(e != Ok, {});

    e = image_create_view(image);
    BALLISTIC_ERR_FAIL_COND_V(e != Ok, {});

    return image;
}

Error DeviceDriverVulkan::image_bind(Image& r_image, VmaAllocation p_allocation)
{
    using enum Error;

    VkResult err = vmaBindImageMemory(allocator, p_allocation, r_image.image);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't bind VMA image memory.");

    r_image.allocation = p_allocation;
    return Ok;
}

Error DeviceDriverVulkan::image_create_view(Image& r_image)
{
    using enum Error;

    VkImageViewCreateInfo view_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_ci.image = r_image.image;
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format = r_image.format;
    view_ci.subresourceRange.aspectMask = r_image.aspect;
    view_ci.subresourceRange.baseMipLevel = 0;
    view_ci.subresourceRange.levelCount = r_image.mip_levels;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.layerCount = r_image.layers;

    VkResult err = vkCreateImageView(device, &view_ci, nullptr, &r_image.image_view);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan image view.");

    return Ok;
}

void DeviceDriverVulkan::image_free(Image& r_image)
{
    if (r_image.image_view) {
        vkDestroyImageView(device, r_image.image_view, nullptr);
        r_image.image_view = VK_NULL_HANDLE;
    }
    if (r_image.allocation) {
        vmaDestroyImage(allocator, r_image.image, r_image.allocation);
        r_image.image= VK_NULL_HANDLE;
        r_image.allocation = nullptr;
    }
    r_image.state = {};
}

/*****************/
/**** SAMPLER ****/
/*****************/

VkSampler DeviceDriverVulkan::sampler_create(const SamplerCreateInfo& p_create_info)
{
    using enum Error;

    VkSamplerCreateInfo sampler_ci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_ci.magFilter = p_create_info.filter;
    sampler_ci.minFilter = p_create_info.filter;
    sampler_ci.addressModeU = p_create_info.address_mode;
    sampler_ci.addressModeV = p_create_info.address_mode;
    sampler_ci.addressModeW = p_create_info.address_mode;
    sampler_ci.anisotropyEnable = p_create_info.anisotropy > 1.0f;
    sampler_ci.maxAnisotropy = p_create_info.anisotropy;
    sampler_ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_ci.unnormalizedCoordinates = false;
    sampler_ci.compareEnable = p_create_info.compare;
    sampler_ci.compareOp = p_create_info.compare_op;
    sampler_ci.mipmapMode = p_create_info.mipmap_mode;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = VK_LOD_CLAMP_NONE;

    VkSampler sampler;
    VkResult err = vkCreateSampler(device, &sampler_ci, nullptr, &sampler);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't create Vulkan sampler.");

    set_object_name(VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, p_create_info.name);
    return sampler;
}

void DeviceDriverVulkan::sampler_free(VkSampler& r_sampler)
{
    if (r_sampler) {
        vkDestroySampler(device, r_sampler, nullptr);
        r_sampler = VK_NULL_HANDLE;
    }
}

/*******************/
/**** SWAPCHAIN ****/
/*******************/

bool DeviceDriverVulkan::_determine_swapchain_format(ContextDriverVulkan::Surface* surface, VkSurfaceFormatKHR &r_surface_format)
{    
    std::vector<VkSurfaceFormatKHR> surface_formats;
    uint32_t format_count = 0;
    VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface->surface, &format_count, nullptr);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, "Couldn't get Vulkan surface present modes.");

	surface_formats.resize(format_count);
	err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface->surface, &format_count, surface_formats.data());
	BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, "Couldn't get Vulkan surface present modes.");

    VkSurfaceFormatKHR surface_format = surface_formats[0];
    for (const auto& f : surface_formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surface_format = f;
            break;
        }
    }
    r_surface_format = surface_format;

    return true;
}

void DeviceDriverVulkan::_swapchain_release()
{
    for (Image& img : swapchain.images) {
        if (img.image_view) vkDestroyImageView(device, img.image_view, nullptr);
        img.image_view = VK_NULL_HANDLE;
    }
    swapchain.images.clear();

    swapchain.image_index = UINT_MAX;

    if (swapchain.swapchain) {
        vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
        swapchain.swapchain = VK_NULL_HANDLE;
    }

    for (VkSemaphore semaphore : swapchain.present_semaphores) {
        semaphore_free(semaphore);
    }
    swapchain.present_semaphores.clear();
}

Error DeviceDriverVulkan::swapchain_create(ContextDriverVulkan::Surface* r_surface)
{
    using enum Error;
    BALLISTIC_ERR_FAIL_COND_V(!r_surface, Failed);
    swapchain.surface = r_surface;
    return Ok;
}

Error DeviceDriverVulkan::swapchain_resize(uint32_t p_desired_framebuffer_count)
{
    using enum Error;
    
    _swapchain_release();

    ContextDriverVulkan::Surface* surface = (ContextDriverVulkan::Surface*)(swapchain.surface);
    VkSurfaceCapabilitiesKHR surface_capabilities = {};
    VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface->surface, &surface_capabilities);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan surface capabilities.");

    if (!swapchain.swapchain) {
        if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
			surface_capabilities.currentExtent.width = std::clamp(surface->width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			surface_capabilities.currentExtent.height = std::clamp(surface->height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}
    }

    VkExtent2D extent;
    if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
        extent.width = std::clamp(surface->width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		extent.height = std::clamp(surface->height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    } else {
        extent = surface_capabilities.currentExtent;
        surface->width = extent.width;
        surface->height = extent.height;
    }

	if (surface->width == 0 || surface->height == 0) {
		return Failed;
	}

    std::vector<VkPresentModeKHR> present_modes;
    uint32_t present_modes_count = 0;
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface->surface, &present_modes_count, nullptr);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan surface present modes.");

	present_modes.resize(present_modes_count);
	err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface->surface, &present_modes_count, present_modes.data());
	BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan surface present modes.");

    VkPresentModeKHR present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
	std::string present_mode_name = "Enabled";
    if (surface->vsync_enabled) {
        present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
		present_mode_name = "Mailbox";
    } else {
        present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		present_mode_name = "Disabled";
    }

    bool present_mode_available = false;
    for (auto mode : present_modes) {
        if (mode == present_mode) present_mode_available = true;
    }

	if (!present_mode_available) {
		surface->vsync_enabled = true;
		present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	uint32_t desired_swapchain_images = std::max(p_desired_framebuffer_count, surface_capabilities.minImageCount);
	if (surface_capabilities.maxImageCount > 0) {
		desired_swapchain_images = std::min(desired_swapchain_images, surface_capabilities.maxImageCount);
	}

    VkSurfaceFormatKHR surface_format{};
    if (!_determine_swapchain_format(surface, surface_format)) {
        BALLISTIC_ERR_FAIL_COND_V_MSG(true, Failed, "Vulkan surface did not return any valid formats.");
    } else {
        swapchain.format = surface_format.format;
        swapchain.color_space = surface_format.colorSpace;
    }

    VkSwapchainCreateInfoKHR swap_ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swap_ci.surface = surface->surface;
    swap_ci.minImageCount = desired_swapchain_images;
    swap_ci.imageFormat = swapchain.format;
    swap_ci.imageColorSpace = swapchain.color_space;
    swap_ci.imageExtent = extent;
    swap_ci.imageArrayLayers = 1;
    swap_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_ci.preTransform = surface_capabilities.currentTransform;
    swap_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_ci.presentMode = present_mode;
    swap_ci.clipped = VK_TRUE;

    err = vkCreateSwapchainKHR(device, &swap_ci, nullptr, &swapchain.swapchain);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan swapchain.");
    
    uint32_t image_count = 0;
    err = vkGetSwapchainImagesKHR(device, swapchain.swapchain, &image_count, nullptr);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan swapchain images.");
    
    std::vector<VkImage> raw_images(image_count);
    err = vkGetSwapchainImagesKHR(device, swapchain.swapchain, &image_count, raw_images.data());
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get Vulkan swapchain images.");

    VkImageViewCreateInfo view_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format = swapchain.format;
    view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
    view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_ci.subresourceRange.levelCount = 1;
    view_ci.subresourceRange.layerCount = 1;

    swapchain.images.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
        Image& img = swapchain.images[i];
        img = {};
        img.image = raw_images[i];
        img.format = swapchain.format;
        img.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        img.extent = { extent.width, extent.height, 1 };
        img.allocation = nullptr;
        img.state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        img.state.stage  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        img.state.access = 0;

        view_ci.image = img.image;
        err = vkCreateImageView(device, &view_ci, nullptr, &img.image_view);
        BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan image view for swapchain image.");
    }

	VkSemaphore semaphore = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < image_count; i++) {
        semaphore = semaphore_create();
		swapchain.present_semaphores.push_back(semaphore);
	}

    swapchain.surface->needs_resize = false;
    return Ok;
}

void DeviceDriverVulkan::swapchain_free()
{
    _swapchain_release();
}

Error DeviceDriverVulkan::swapchain_acquire_next_image(VkSemaphore p_signal_semaphore)
{
    using enum Error;

    VkResult err = vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX, p_signal_semaphore, VK_NULL_HANDLE, &swapchain.image_index);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        log_write("NEEDS_RESIZE from acquire OUT_OF_DATE or SUBOPTIMAL_KHR");
        swapchain.surface->needs_resize = true;
        return Ok;
    }
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't get next Vulkan swapchain image.");
    
    return Ok;
    
}

Error DeviceDriverVulkan::swapchain_update()
{
    using enum Error;
    if (!swapchain.surface->needs_resize) return Ok;
    device_wait_idle();
    return swapchain_resize(frame_count);
}

/*********************/
/**** DESCRIPTORS ****/
/*********************/

// ----- BINDLESS HEAP -----

Error DeviceDriverVulkan::bindless_heap_create(uint32_t p_sampled_count, uint32_t p_storage_count, uint32_t p_samplers_count)
{
    using enum Error;
    
    VkPhysicalDeviceVulkan12Properties p12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
    VkPhysicalDeviceProperties2 p2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    p2.pNext = &p12;
    vkGetPhysicalDeviceProperties2(physical_device, &p2);

    auto clamp = [](uint32_t want, uint32_t per_set, uint32_t per_stage) {
        uint32_t lim = per_set < per_stage ? per_set : per_stage;
        return want < lim ? want : lim;
    };
    const uint32_t sampled = clamp(p_sampled_count, p12.maxDescriptorSetUpdateAfterBindSampledImages, p12.maxPerStageDescriptorUpdateAfterBindSampledImages);
    const uint32_t storage = clamp(p_storage_count, p12.maxDescriptorSetUpdateAfterBindStorageImages, p12.maxPerStageDescriptorUpdateAfterBindStorageImages);
    const uint32_t samplers = clamp(p_samplers_count, p12.maxDescriptorSetUpdateAfterBindSamplers,      p12.maxPerStageDescriptorUpdateAfterBindSamplers);

    if (sampled < p_sampled_count || storage < p_storage_count || samplers < p_samplers_count) {
        log_write("Bindless heap counts clamped to device limits (sampled %u->%u, storage %u->%u, sampler %u->%u).\n", p_sampled_count, sampled, p_storage_count, storage, p_samplers_count, samplers);
    }

    bindless_heap.sampled_alloc.cap = sampled;
    bindless_heap.storage_alloc.cap = storage;

    VkDescriptorSetLayoutBinding bindings[3]{};
    bindings[BindlessHeap::BINDING_SAMPLED] = { BindlessHeap::BINDING_SAMPLED, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampled, VK_SHADER_STAGE_ALL, nullptr };
    bindings[BindlessHeap::BINDING_STORAGE] = { BindlessHeap::BINDING_STORAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storage, VK_SHADER_STAGE_ALL, nullptr };
    bindings[BindlessHeap::BINDING_SAMPLER] = { BindlessHeap::BINDING_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER, samplers, VK_SHADER_STAGE_ALL, nullptr };

    const VkDescriptorBindingFlags bf = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorBindingFlags flags[3]{ bf, bf, bf};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_ci{};
    flags_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flags_ci.bindingCount = 3;
    flags_ci.pBindingFlags = flags;

    VkDescriptorSetLayoutCreateInfo layout_ci{};
    layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_ci.pNext = &flags_ci;
    layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layout_ci.bindingCount = 3;
    layout_ci.pBindings = bindings;

    VkResult err = vkCreateDescriptorSetLayout(device, &layout_ci, nullptr, &bindless_heap.layout);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan descriptor set layout.");

    VkDescriptorPoolSize pool_sizes[3] = {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampled },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storage },
        { VK_DESCRIPTOR_TYPE_SAMPLER, samplers },
    };
    VkDescriptorPoolCreateInfo pool_ci{};
    pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = 3;
    pool_ci.pPoolSizes = pool_sizes;

    err = vkCreateDescriptorPool(device, &pool_ci, nullptr, &bindless_heap.pool);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan descriptor pool.");

    VkDescriptorSetAllocateInfo set_ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    set_ai.descriptorPool = bindless_heap.pool;
    set_ai.descriptorSetCount = 1;
    set_ai.pSetLayouts = &bindless_heap.layout;

    err = vkAllocateDescriptorSets(device, &set_ai, &bindless_heap.set);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan descriptor set.");

    VkPushConstantRange push_range{};
    push_range.stageFlags = VK_SHADER_STAGE_ALL;
    push_range.offset = 0;
    push_range.size = BindlessHeap::PUSH_CONSTANT_SIZE;

    VkPipelineLayoutCreateInfo pl_ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pl_ci.setLayoutCount = 1;
    pl_ci.pSetLayouts = &bindless_heap.layout;
    pl_ci.pushConstantRangeCount = 1;
    pl_ci.pPushConstantRanges = &push_range;

    err = vkCreatePipelineLayout(device, &pl_ci, nullptr, &bindless_heap.pipeline_layout);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, Failed, "Couldn't create Vulkan pipeline layout.");

    return Ok;
}

void DeviceDriverVulkan::bindless_heap_free()
{
    if (bindless_heap.pipeline_layout) {
        vkDestroyPipelineLayout(device, bindless_heap.pipeline_layout, nullptr);
        bindless_heap.pipeline_layout = VK_NULL_HANDLE;
    }
    if (bindless_heap.pool) {
        vkDestroyDescriptorPool(device, bindless_heap.pool, nullptr);
        bindless_heap.pool = VK_NULL_HANDLE;
    }
    if (bindless_heap.layout) {
        vkDestroyDescriptorSetLayout(device, bindless_heap.layout, nullptr);
        bindless_heap.layout = VK_NULL_HANDLE;
    }
    bindless_heap.set = VK_NULL_HANDLE;
}

uint32_t DeviceDriverVulkan::bindless_heap_alloc_sampled(VkImageView p_image_view)
{
    uint32_t index = bindless_heap.sampled_alloc.acquire();
    if (index == UINT32_MAX) {
        
        log_write("Bindless heap sampled image array exhausted.");
        return UINT32_MAX;
    }

    VkDescriptorImageInfo info{};
    info.imageView = p_image_view;
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w.dstSet = bindless_heap.set;
    w.dstBinding = BindlessHeap::BINDING_SAMPLED;
    w.dstArrayElement = index;
    w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    w.descriptorCount = 1;
    w.pImageInfo = &info;

    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
    return index;
}

uint32_t DeviceDriverVulkan::bindless_heap_alloc_storage(VkImageView p_image_view)
{
    uint32_t index = bindless_heap.storage_alloc.acquire();
    if (index == UINT32_MAX) {
        log_write("Bindless heap storage image array exhausted.");
        return UINT32_MAX;
    }

    VkDescriptorImageInfo info{};
    info.imageView = p_image_view;
    info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w.dstSet = bindless_heap.set;
    w.dstBinding = BindlessHeap::BINDING_STORAGE;
    w.dstArrayElement = index;
    w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    w.descriptorCount = 1;
    w.pImageInfo = &info;

    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
    return index;
}

void DeviceDriverVulkan::bindless_heap_free_sampled(uint32_t p_index)
{
    if (p_index == UINT32_MAX) return;
    bindless_heap.sampled_alloc.release(p_index);
}

void DeviceDriverVulkan::bindless_heap_free_storage(uint32_t p_index)
{
    if (p_index == UINT32_MAX) return;
    bindless_heap.storage_alloc.release(p_index);
}

void DeviceDriverVulkan::bindless_heap_register_sampler(uint32_t p_index, VkSampler p_sampler)
{
    VkDescriptorImageInfo info{};
    info.sampler = p_sampler;

    VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w.dstSet = bindless_heap.set;
    w.dstBinding = BindlessHeap::BINDING_SAMPLER;
    w.dstArrayElement = p_index;
    w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    w.descriptorCount = 1;
    w.pImageInfo = &info;

    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
}

/******************/
/**** PIPELINE ****/
/******************/

// ----- CACHE -----

// ----- PIPELINE -----

DeviceDriverVulkan::Pipeline DeviceDriverVulkan::graphics_pipeline_create(const GraphicsPipelineCreateInfo& p_create_info)
{
    using enum Error;

    Pipeline pipeline;
    pipeline.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkPipelineShaderStageCreateInfo stages[2]{};
    uint32_t stage_count = 0;
    if (p_create_info.vertex_shader) {
        VkPipelineShaderStageCreateInfo& s = stages[stage_count++];
        s.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        s.stage = VK_SHADER_STAGE_VERTEX_BIT;
        s.module = p_create_info.vertex_shader;
        s.pName = "main";
    }
    if (p_create_info.fragment_shader) {
        VkPipelineShaderStageCreateInfo& s = stages[stage_count++];
        s.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        s.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        s.module = p_create_info.fragment_shader;
        s.pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo vertex_input{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    VkPipelineInputAssemblyStateCreateInfo input_assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    input_assembly.topology = p_create_info.topology;

    VkPipelineViewportStateCreateInfo viewport_state{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    raster.polygonMode = p_create_info.polygon_mode;
    raster.cullMode = p_create_info.cull_mode;
    raster.frontFace = p_create_info.front_face;
    raster.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisample.rasterizationSamples = p_create_info.samples;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depth_stencil.depthTestEnable = p_create_info.depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = p_create_info.depth_write ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = p_create_info.depth_compare;
    depth_stencil.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState blend_attachments[8]{};
    for (uint32_t i = 0; i < p_create_info.color_attachment_count; i++) {
        // const ColorBlendState& b = p_create_info.blend[i];
        // VkPipelineColorBlendAttachmentState& a = blend_attachments[i];
        // a.blendEnable = b.blend_enable ? VK_TRUE : VK_FALSE;
        // a.srcColorBlendFactor = b.src_color;
        // a.dstColorBlendFactor = b.dst_color;
        // a.colorBlendOp = b.color_op;
        // a.srcAlphaBlendFactor = b.src_alpha;
        // a.dstAlphaBlendFactor = b.dst_alpha;
        // a.alphaBlendOp = b.alpha_op;
        // a.colorWriteMask = b.write_mask;
    }

    VkPipelineColorBlendStateCreateInfo color_blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    color_blend.attachmentCount = p_create_info.color_attachment_count;
    color_blend.pAttachments = blend_attachments;

    VkDynamicState dyn[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic.dynamicStateCount = 2;
    dynamic.pDynamicStates = dyn;

    VkPipelineRenderingCreateInfo rendering{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    rendering.colorAttachmentCount = p_create_info.color_attachment_count;
    rendering.pColorAttachmentFormats = p_create_info.color_formats;
    rendering.depthAttachmentFormat = p_create_info.depth_format;
    rendering.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo pipeline_ci{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipeline_ci.pNext = &rendering;
    pipeline_ci.stageCount = stage_count;
    pipeline_ci.pStages = stages;
    pipeline_ci.pVertexInputState = &vertex_input;
    pipeline_ci.pInputAssemblyState = &input_assembly;
    pipeline_ci.pViewportState = &viewport_state;
    pipeline_ci.pRasterizationState = &raster;
    pipeline_ci.pMultisampleState = &multisample;
    pipeline_ci.pDepthStencilState = &depth_stencil;
    pipeline_ci.pColorBlendState = &color_blend;
    pipeline_ci.pDynamicState = &dynamic;
    pipeline_ci.layout = bindless_heap.pipeline_layout;
    pipeline_ci.renderPass = VK_NULL_HANDLE;

    VkResult err = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline.pipeline);
    BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't create Vulkan graphics pipeline.");

    set_object_name(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline.pipeline, p_create_info.name);
    return pipeline;
}

DeviceDriverVulkan::Pipeline DeviceDriverVulkan::compute_pipeline_create(const ComputePipelineCreateInfo& p_create_info)
{
    using enum Error;

    Pipeline pipeline;
    pipeline.bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;

    (void)p_create_info;

    // VkResult err = vkCreatePipeline(device, &pipeline_ci, nullptr, &pipeline.pipeline);
    // BALLISTIC_ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, {}, "Couldn't create Vulkan compute pipeline.");

    // set_object_name(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline.pipeline, p_create_info.name);
    return pipeline;
}

void DeviceDriverVulkan::pipeline_destroy(Pipeline& r_pipeline)
{
    if (r_pipeline.pipeline) {
        vkDestroyPipeline(device, r_pipeline.pipeline, nullptr);
        r_pipeline.pipeline = VK_NULL_HANDLE;
    }

}

/***************/
/**** UTILS ****/
/***************/

void DeviceDriverVulkan::set_object_name(VkObjectType p_type, uint64_t p_handle, const char* p_name)
{
    if (!p_name) return;
    auto fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (!fn) return;
    VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    info.objectType = p_type;
    info.objectHandle = p_handle;
    info.pObjectName = p_name;
    fn(device, &info);
}

}