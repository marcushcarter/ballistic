#include <vk/core/device.h>
// #include "graphics/vk/misc/utils.h"
#include <set>

bool Device::Create(VkPhysicalDevice physicalDevice, uint32_t graphicsFamily, uint32_t presentFamily, uint32_t transferFamily, const std::vector<const char*>& requiredExtensions)
{
    Destroy();

    std::set<uint32_t> uniqueFamilies = { graphicsFamily, presentFamily, transferFamily };
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    float priority = 1.0f;

    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
        queueInfos.push_back(info);
    }

    VkPhysicalDeviceFeatures2 supportedFeatures{};
    supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    VkPhysicalDeviceVulkan12Features supported12{};
    supported12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    VkPhysicalDeviceVulkan13Features supported13{};
    supported13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    supportedFeatures.pNext = &supported12;
    supported12.pNext = &supported13;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &supportedFeatures);

    auto Require = [](VkBool32 feature, const char* name) {
        if (!feature) {
            (void)name;
            // LOG_ERROR("Required Vulkan feature not supported: %s", name);
            return false;
        }
        return true;
    };

    if (!Require(supportedFeatures.features.samplerAnisotropy, "samplerAnisotropy")) return false;
    if (!Require(supportedFeatures.features.multiDrawIndirect, "multiDrawIndirect")) return false;

    if (!Require(supported12.descriptorIndexing, "descriptorIndexing")) return false;
    if (!Require(supported12.runtimeDescriptorArray, "runtimeDescriptorArray")) return false;

    if (!Require(supported12.descriptorBindingPartiallyBound, "descriptorBindingPartiallyBound")) return false;
    if (!Require(supported12.descriptorBindingSampledImageUpdateAfterBind, "descriptorBindingSampledImageUpdateAfterBind")) return false;
    if (!Require(supported12.descriptorBindingStorageImageUpdateAfterBind, "descriptorBindingStorageImageUpdateAfterBind")) return false;
    if (!Require(supported12.descriptorBindingVariableDescriptorCount, "descriptorBindingVariableDescriptorCount")) return false;
    if (!Require(supported12.shaderSampledImageArrayNonUniformIndexing, "shaderSampledImageArrayNonUniformIndexing")) return false;
    if (!Require(supported12.shaderStorageImageArrayNonUniformIndexing, "shaderStorageImageArrayNonUniformIndexing")) return false;

    if (!Require(supported13.dynamicRendering, "dynamicRendering")) return false;
    if (!Require(supported13.synchronization2, "synchronization2")) return false;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.features.samplerAnisotropy = VK_TRUE;
    features2.features.multiDrawIndirect = VK_TRUE;
    if (supportedFeatures.features.wideLines) features2.features.wideLines = VK_TRUE;
    if (supportedFeatures.features.geometryShader) features2.features.geometryShader = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;
    if (supported13.shaderDemoteToHelperInvocation) features13.shaderDemoteToHelperInvocation = VK_TRUE;
    features2.pNext = &features12;
    features12.pNext = &features13;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &features2;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan device");
        return false;
    }
    
    // SetObjectName(device, VK_OBJECT_TYPE_DEVICE, (uint64_t)device, "Device");
    // LOG_DEBUG("Logical Device created");
    return true;
}

void Device::Destroy()
{
    if (device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
        // LOG_DEBUG("Logical Device destroyed");
    }
}

void Device::Wait() { vkDeviceWaitIdle(device); }
