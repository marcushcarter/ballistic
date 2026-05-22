#pragma once
#include "pch.h"

inline VkPushConstantRange PushConstant(VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT, uint32_t offset = 0, uint32_t size = 0) {
    VkPushConstantRange p{};
    p.stageFlags = stage;
    p.offset = offset;
    p.size = size;
    return p;
}

struct PipelineLayout
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    bool Create(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants);
    void Destroy();

    VkPipelineLayout& Get() { return pipelineLayout; }

private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};