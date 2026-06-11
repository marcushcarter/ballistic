#include <image/sampler.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool Sampler::Create(VkDevice device, const SamplerDesc& d)
{
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = d.magFilter;
    createInfo.minFilter = d.minFilter;
    createInfo.addressModeU = d.addressModeU;
    createInfo.addressModeV = d.addressModeV;
    createInfo.addressModeW = d.addressModeW;
    createInfo.anisotropyEnable = d.anisotropyEnable ? VK_TRUE : VK_FALSE;
    createInfo.maxAnisotropy = d.maxAnisotropy;
    createInfo.borderColor = d.borderColor;
    createInfo.unnormalizedCoordinates = d.unnormalizedCoordinates ? VK_TRUE : VK_FALSE;
    createInfo.compareEnable = d.compareEnable ? VK_TRUE : VK_FALSE;
    createInfo.compareOp = d.compareOp;
    createInfo.mipmapMode = d.mipmapMode;
    createInfo.mipLodBias = d.mipLodBias;
    createInfo.minLod = d.minLod;
    createInfo.maxLod = d.maxLod;

    if (vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS) {
        // LOG_ERROR("Sampler create failed: %s - vkCreateSampler", debugName ? debugName : "Unnamed");
        return false;
    }
    
    SetObjectName(device, VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, d.debugName);
    // LOG_DEBUG("Sampler created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void Sampler::Destroy()
{
    if (sampler) {
        vkDestroySampler(deviceHandle, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
        // LOG_DEBUG("Sampler destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
}