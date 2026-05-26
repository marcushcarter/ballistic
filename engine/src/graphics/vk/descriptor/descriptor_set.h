#pragma once
#include "pch.h"

struct DescriptorSetDesc
{
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const char* debugName = nullptr;
};

struct DescriptorSet
{
    VkDescriptorSet set = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    bool Allocate(VkDevice device, const DescriptorSetDesc& desc);

    void SetBuffers(uint32_t binding, VkDescriptorType type, const std::vector<VkBuffer>& buffers, VkDeviceSize range = VK_WHOLE_SIZE, uint32_t dstArrayElement = 0);
    void SetImages(uint32_t binding, const std::vector<VkImageView>& views, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t dstArrayElement = 0);
    void SetStorageImages(uint32_t binding, const std::vector<VkImageView>& views, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL, uint32_t dstArrayElement = 0);

    VkDescriptorSet Get() const { return set; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
