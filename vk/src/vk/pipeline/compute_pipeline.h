#pragma once
#include <vk/pipeline/pipeline.h>

struct ComputePipelineDesc
{
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipelineCache cache = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo shaderStage;
    const char* debugName = nullptr;
};

struct ComputePipeline : Pipeline
{
    ComputePipeline() : Pipeline(VK_PIPELINE_BIND_POINT_COMPUTE) {}
    bool Create(VkDevice device, const ComputePipelineDesc& desc);

    void Dispatch(VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
    void DispatchIndirect(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset = 0);
};