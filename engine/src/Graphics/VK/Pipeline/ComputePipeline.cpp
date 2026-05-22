#include "ComputePipeline.h"

bool ComputePipeline::Create(VkDevice device, VkPipelineLayout layout, VkPipelineShaderStageCreateInfo shaderStage, VkPipelineCache pipelineCache)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(layout, VkPipelineLayout);

    Destroy();
    deviceHandle = device;
    pipelineLayoutHandle = layout;

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = shaderStage;
    createInfo.layout = layout;

    if (vkCreateComputePipelines(device, pipelineCache, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan compute pipeline");
        return false;
    }

    LOG_DEBUG("Compute Pipeline created");
    return true;
}

void ComputePipeline::Dispatch(VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    vkCmdDispatch(cmd, groupCountX, groupCountY, groupCountZ);
}

void ComputePipeline::DispatchIndirect(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset)
{
    vkCmdDispatchIndirect(cmd, buffer, offset);
}
