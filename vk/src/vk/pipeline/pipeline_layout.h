#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

inline VkPushConstantRange PushConstant(VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT, uint32_t offset = 0, uint32_t size = 0) {
    VkPushConstantRange p{};
    p.stageFlags = stage;
    p.offset = offset;
    p.size = size;
    return p;
}

struct PipelineLayoutDesc
{
    std::vector<VkDescriptorSetLayout> setLayouts;
    std::vector<VkPushConstantRange> pushConstants;
    const char* debugName = nullptr;
};

struct PipelineLayout
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    const char* debugName = nullptr;

    bool Create(VkDevice device, const PipelineLayoutDesc& desc);
    void Destroy();

    VkPipelineLayout& Get() { return pipelineLayout; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};