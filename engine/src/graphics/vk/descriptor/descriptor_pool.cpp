#include "descriptor_pool.h"

bool DescriptorPool::Create(VkDevice device, const DescriptorPoolDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);
    
    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;
    
    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    auto add = [&](VkDescriptorType type, uint32_t count) {
        if (count > 0) {
            poolSizes.push_back({ type, count });
            maxSets += count;
        }
    };

    add(VK_DESCRIPTOR_TYPE_SAMPLER, desc.samplers);
    add(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, desc.combinedImageSamplers);
    add(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, desc.sampledImages);
    add(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, desc.storageImages);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, desc.uniformTexelBuffers);
    add(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, desc.storageTexelBuffers);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, desc.uniformBuffers);
    add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, desc.storageBuffers);
    add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, desc.uniformBuffersDynamic);
    add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, desc.storageBuffersDynamic);
    add(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, desc.inputAttachments);

    if (poolSizes.empty()) {
        LOG_ERROR("Descriptor Pool create failed: %s - no pool sizes specified", debugName ? debugName : "Unnamed");
        return false;
    }

    VkDescriptorPoolCreateFlags flags = 0;
    if (desc.freeable)  flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    if (desc.afterBind) flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = flags;
    createInfo.maxSets = maxSets;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(deviceHandle, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        LOG_ERROR("Descriptor Pool create failed: %s - vkCreateDescriptorPool", debugName ? debugName : "Unnamed");
        return false;
    }

    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        nameInfo.objectHandle = (uint64_t)descriptorPool;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }

    LOG_DEBUG("Descriptor Pool created: %s (%u max sets)", debugName ? debugName : "Unnamed", maxSets);
    return true;
}

void DescriptorPool::Destroy()
{
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(deviceHandle, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Descriptor Pool destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
