#include <sync/fence.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool Fence::Create(VkDevice device, bool signaled, const char* name)
{
    Destroy();
    debugName = name;
    deviceHandle = device;

    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if (vkCreateFence(device, &createInfo, nullptr, &fence) != VK_SUCCESS) {
        // LOG_ERROR("Fence create failed: %s - vkCreateFence", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_FENCE, (uint64_t)fence, debugName);
    // LOG_DEBUG("Fence created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void Fence::Destroy()
{
    if (fence) {
        vkDestroyFence(deviceHandle, fence, nullptr);
        fence = VK_NULL_HANDLE;
        // LOG_DEBUG("Fence destroyed: %s", debugName ? debugName : "Unnamed");
    }
}

void Fence::Wait(uint64_t timeout) { vkWaitForFences(deviceHandle, 1, &fence, VK_TRUE, timeout); }

void Fence::Reset() { vkResetFences(deviceHandle, 1, &fence); }
}