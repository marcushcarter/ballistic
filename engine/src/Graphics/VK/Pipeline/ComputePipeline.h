#pragma once
#include "pch.h"
#include "Pipeline.h"

struct ComputePipeline : Pipeline
{
    ComputePipeline() : Pipeline(VK_PIPELINE_BIND_POINT_COMPUTE) {}

    bool Create(VkDevice device, VkPipelineLayout layout, VkPipelineShaderStageCreateInfo shaderStage, VkPipelineCache pipelineCache);

    void Dispatch(VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
    void DispatchIndirect(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset = 0);
};