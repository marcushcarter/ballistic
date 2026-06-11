#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace Ballistic::Vulkan
{
struct SamplerDesc {
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    float mipLodBias = 0.0f;
    bool anisotropyEnable = false;
    float maxAnisotropy = 1.0f;
    bool compareEnable = false;
    VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
    float minLod = 0.0f;
    float maxLod = VK_LOD_CLAMP_NONE;
    VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    bool unnormalizedCoordinates = false;
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    const char* debugName = nullptr;
};

struct Sampler
{
    VkSampler sampler = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    uint32_t bindlessSampler = UINT32_MAX;

    bool Create(VkDevice device, const SamplerDesc& desc);
    void Destroy();

    VkSampler Get() const { return sampler; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
}