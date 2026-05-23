#include "Sampler.h"

bool Sampler::Create(VkDevice device, const SamplerDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = desc.magFilter;
    createInfo.minFilter = desc.minFilter;
    createInfo.addressModeU = desc.addressModeU;
    createInfo.addressModeV = desc.addressModeV;
    createInfo.addressModeW = desc.addressModeW;
    createInfo.anisotropyEnable = desc.anisotropyEnable ? VK_TRUE : VK_FALSE;
    createInfo.maxAnisotropy = desc.maxAnisotropy;
    createInfo.borderColor = desc.borderColor;
    createInfo.unnormalizedCoordinates = desc.unnormalizedCoordinates ? VK_TRUE : VK_FALSE;
    createInfo.compareEnable = desc.compareEnable ? VK_TRUE : VK_FALSE;
    createInfo.compareOp = desc.compareOp;
    createInfo.mipmapMode = desc.mipmapMode;
    createInfo.mipLodBias = desc.mipLodBias;
    createInfo.minLod = desc.minLod;
    createInfo.maxLod = desc.maxLod;

    if (vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan sampler");
        return false;
    }
    
    if (debugName) {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_SAMPLER;
        nameInfo.objectHandle = (uint64_t)sampler;
        nameInfo.pObjectName = debugName;
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        if (func) func(device, &nameInfo);
    }
    
    LOG_DEBUG("Sampler created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void Sampler::Destroy()
{
    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(deviceHandle, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
        LOG_DEBUG("Sampler destroyed");
    }
    deviceHandle = VK_NULL_HANDLE;
}
