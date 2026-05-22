#include "Pipeline.h"

void Pipeline::Destroy()
{
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(deviceHandle, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        pipelineLayoutHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Pipeline destroyed");
    }
}

void Pipeline::Bind(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, bindPoint, pipeline);
}

void Pipeline::DescriptorSets(VkCommandBuffer cmd, std::vector<VkDescriptorSet> sets, uint32_t setIndex)
{
    if (sets.empty()) return;
    vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayoutHandle, setIndex, (uint32_t)sets.size(), sets.data(), 0, nullptr);
}

void Pipeline::PushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
{
    if (data && size > 0)
        vkCmdPushConstants(cmd, pipelineLayoutHandle, stageFlags, offset, size, data);
}