#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/error/error_macros.h>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <algorithm>

namespace ballistic::drivers {

void RenderingDeviceDriverVulkan::_register_requested_device_extension(const std::string& p_extension_name, bool p_required) {
    requested_device_extensions[p_extension_name] = p_required;
}

Error RenderingDeviceDriverVulkan::_initialize_device_extensions()
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

void RenderingDeviceDriverVulkan::_get_device_properties()
{
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    
    std::cout << "Vulkan " << VK_API_VERSION_MAJOR(physical_device_properties.apiVersion) << "." << VK_API_VERSION_MINOR(physical_device_properties.apiVersion) << "." << VK_API_VERSION_PATCH(physical_device_properties.apiVersion)
        << " - Driver " << VK_API_VERSION_MAJOR(physical_device_properties.driverVersion) << "." << VK_API_VERSION_MINOR(physical_device_properties.driverVersion) << "." << VK_API_VERSION_PATCH(physical_device_properties.driverVersion)
        << " - Using Device: " << physical_device_properties.deviceName << " (" << vk::to_string(vk::PhysicalDeviceType(physical_device_properties.deviceType)).c_str() << ")\n";
}

Error RenderingDeviceDriverVulkan::_check_device_features()
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

void RenderingDeviceDriverVulkan::_check_subgroup_capabilities()
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

Error RenderingDeviceDriverVulkan::_check_device_capabilities()
{
    using enum Error;

    _check_subgroup_capabilities();

    return Ok;
}

Error RenderingDeviceDriverVulkan::_add_queue_create_info(std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info)
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
        p_queue_create_info.push_back(create_info);
        queue_families[family_index].resize(1);
    }

    return Ok;
}

Error RenderingDeviceDriverVulkan::_initialize_device(const std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info)
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
// 		memory_report_info.pfnUserCallback = RenderingContextDriverVulkan::memory_report_callback;
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

Error RenderingDeviceDriverVulkan::_initialize_allocator()
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

Error RenderingDeviceDriverVulkan::_initialize_pipeline_cache()
{
    using enum Error;

    return Ok;
}

Error RenderingDeviceDriverVulkan::initialize(uint32_t p_device_index, uint32_t p_frame_count)
{
    using enum Error;
    Error err;

    device_index = p_device_index;
    context_device = context_driver->device_get(device_index);
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

    return Ok;
}

}