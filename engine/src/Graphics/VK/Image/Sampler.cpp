#include "Sampler.h"

bool Sampler::Create(VkDevice device, const SamplerDesc& desc)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);

    Destroy();
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

    LOG_DEBUG("Sampler created");
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
