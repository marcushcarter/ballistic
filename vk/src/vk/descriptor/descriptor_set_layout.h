#pragma once
#include <vulkan/vulkan.h>
#include <vector>

inline VkDescriptorSetLayoutBinding SetLayoutBinding(uint32_t binding, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT, uint32_t count = 1) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = binding;
    b.descriptorType = type;
    b.stageFlags = stage;
    b.descriptorCount = count;
    b.pImmutableSamplers = nullptr;
    return b;
}

struct DescriptorSetLayoutDesc
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorBindingFlags> bindingFlags;
    VkDescriptorSetLayoutCreateFlags layoutFlags = 0;
    const char* debugName = nullptr;
};

struct DescriptorSetLayout
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    bool Create(VkDevice device, const DescriptorSetLayoutDesc& desc);
    void Destroy();

    VkDescriptorSetLayout Get() const { return layout; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};
