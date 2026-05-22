#include "Semaphore.h"

bool Semaphore::Create(VkDevice device)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    deviceHandle = device;

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    if (vkCreateSemaphore(device, &createInfo, nullptr, &semaphore) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan semaphore");
        return false;
    }

    LOG_DEBUG("Semaphore created");
    return true;
}

void Semaphore::Destroy()
{
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(deviceHandle, semaphore, nullptr);
        semaphore = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Semaphore destroyed");
    }
}
