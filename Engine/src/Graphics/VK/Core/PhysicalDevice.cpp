#include "PhysicalDevice.h"

bool PhysicalDevice::Pick(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions)
{
    VK_CHECK_HANDLE(instance, VkInstance);
    VK_CHECK_HANDLE(surface, VkSurfaceKHR);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        LOG_ERROR("No Vulkan GPUs found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    int highestScore = -1;
    for (auto& dev : devices) {

        uint32_t extCount;
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extCount);
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, extensions.data());
        
        bool ok = true;
        for (auto req : requiredExtensions) {
            bool found = false;
            for (auto& ext : extensions) if (strcmp(req, ext.extensionName) == 0) { found = true; break; }
            if (!found) { ok = false; break; }
        }
        if (!ok) continue;

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
        std::vector<VkQueueFamilyProperties> families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families.data());

        int gfx = -1, present = -1, transfer = -1, compute = -1;
        for (uint32_t i = 0; i < families.size(); i++) {
            if (gfx < 0 && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) gfx = i;
            
            VkBool32 canPresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &canPresent);
            if (present < 0 && canPresent) present = i;

            if (transfer < 0 && (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) transfer = i;
            if (compute < 0 && (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) compute = i;
        }

        if (transfer < 0) transfer = gfx;
        if (compute < 0) compute = gfx;

        if (gfx < 0 || present < 0) continue;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(dev, &props);
        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 500;
        score += props.limits.maxImageDimension2D;

        if (score > highestScore) {
            highestScore = score;
            physicalDevice = dev;
            graphicsFamily = gfx;
            presentFamily = present;
            transferFamily = transfer;
            computeFamily = compute;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        LOG_ERROR("No suitable GPU found");
        return false;
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memory);

    if (!features.geometryShader) {
        LOG_WARN("GPU does not support geometry shaders");
        // return false;
    }

    uint64_t dedicatedVRAM = 0;
    uint64_t sharedVRAM = 0;
    for (uint32_t i = 0; i < memory.memoryHeapCount; i++) {
        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            dedicatedVRAM += memory.memoryHeaps[i].size;
        else
            sharedVRAM += memory.memoryHeaps[i].size;
    }

    bool isIntegrated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    double vramGB = static_cast<double>(isIntegrated ? dedicatedVRAM + sharedVRAM : dedicatedVRAM) / (1024.0 * 1024.0 * 1024.0);
    const char* vramLabel = isIntegrated ? "Shared RAM" : "VRAM";

    LOG_INFO("Vulkan %d.%d.%d - Driver %d.%d.%d - Using Device: %s (%s) - %.2f GB %s",
        VK_API_VERSION_MAJOR(properties.apiVersion),
        VK_API_VERSION_MINOR(properties.apiVersion),
        VK_API_VERSION_PATCH(properties.apiVersion),
        VK_API_VERSION_MAJOR(properties.driverVersion),
        VK_API_VERSION_MINOR(properties.driverVersion),
        VK_API_VERSION_PATCH(properties.driverVersion),
        properties.deviceName,
        vk::to_string(vk::PhysicalDeviceType(properties.deviceType)).c_str(),
        vramGB,
        vramLabel
    );

    return true;
}
