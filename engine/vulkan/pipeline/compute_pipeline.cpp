#include <pipeline/compute_pipeline.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool ComputePipeline::Create(VkDevice device, const ComputePipelineDesc& d)
{
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;
    pipelineLayoutHandle = d.layout;

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = d.shaderStage;
    createInfo.layout = d.layout;

    if (vkCreateComputePipelines(device, d.cache, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        // LOG_ERROR("Compute Pipeline create failed: %s - vkCreateComputePipelines", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline, d.debugName);
    // LOG_DEBUG("Compute Pipeline created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void ComputePipeline::Dispatch(VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) { vkCmdDispatch(cmd, groupCountX, groupCountY, groupCountZ); }

void ComputePipeline::DispatchIndirect(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset) { vkCmdDispatchIndirect(cmd, buffer, offset); }
}