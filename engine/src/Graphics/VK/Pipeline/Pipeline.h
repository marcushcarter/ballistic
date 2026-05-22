#pragma once
#include "pch.h"

inline VkPipelineShaderStageCreateInfo PipelineShaderStage(VkShaderModule shader, VkShaderStageFlagBits stage, const char* entry = "main") {
    VkPipelineShaderStageCreateInfo ss{};
    ss.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ss.module = shader;
    ss.stage = stage;
    ss.pName = entry;
    return ss;
}

struct Pipeline
{
    VkPipeline pipeline = VK_NULL_HANDLE;

    void Destroy();
    void Bind(VkCommandBuffer cmd);

    void DescriptorSets(VkCommandBuffer cmd, std::vector<VkDescriptorSet> sets, uint32_t setIndex = 0);
    void PushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);

    VkPipeline Get() const { return pipeline; }

protected:
    explicit Pipeline(VkPipelineBindPoint bp) : bindPoint(bp) {}

    VkPipelineBindPoint bindPoint;
    VkDevice deviceHandle = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayoutHandle = VK_NULL_HANDLE;
};