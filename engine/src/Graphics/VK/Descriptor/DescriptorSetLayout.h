#pragma once
#include "pch.h"

inline VkDescriptorSetLayoutBinding SetLayoutBinding(uint32_t binding, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT, uint32_t count = 1) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = binding;
    b.descriptorType = type;
    b.stageFlags = stage;
    b.descriptorCount = count;
    b.pImmutableSamplers = nullptr;
    return b;
}

struct DescriptorSetLayout
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    bool Create(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings, const std::vector<VkDescriptorBindingFlags>& bindingFlags = {}, VkDescriptorSetLayoutCreateFlags layoutFlags = 0);
    void Destroy();

    VkDescriptorSetLayout Get() const { return layout; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
