#include "Semaphore.h"

bool Semaphore::Create(VkDevice device, const char* name)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    debugName = name;
    deviceHandle = device;

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    if (vkCreateSemaphore(device, &createInfo, nullptr, &semaphore) != VK_SUCCESS) {
        LOG_ERROR("Semaphore create failed: %s - vkCreateSemaphore", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        nameInfo.objectHandle = (uint64_t)semaphore;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Semaphore created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void Semaphore::Destroy()
{
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(deviceHandle, semaphore, nullptr);
        semaphore = VK_NULL_HANDLE;
        LOG_DEBUG("Semaphore destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
