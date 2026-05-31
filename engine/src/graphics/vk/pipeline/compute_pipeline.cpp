#include "compute_pipeline.h"
#include "graphics/vk/misc/utils.h"

bool ComputePipeline::Create(VkDevice device, const ComputePipelineDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);
    VK_CHECK_HANDLE(desc.layout, VkPipelineLayout);

    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;
    pipelineLayoutHandle = desc.layout;

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = desc.shaderStage;
    createInfo.layout = desc.layout;

    if (vkCreateComputePipelines(device, desc.cache, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        LOG_ERROR("Compute Pipeline create failed: %s - vkCreateComputePipelines", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline, debugName);
    LOG_DEBUG("Compute Pipeline created: %s", debugName ? debugName : "Unnamed");
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
