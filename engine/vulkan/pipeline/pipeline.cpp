#include <pipeline/pipeline.h>

namespace Ballistic::Vulkan
{
void Pipeline::Destroy()
{
    if (pipeline) {
        vkDestroyPipeline(deviceHandle, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
        // LOG_DEBUG("Pipeline destroyed: %s", debugName ? debugName : "Unnamed");
    }
}

void Pipeline::Bind(VkCommandBuffer cmd) { vkCmdBindPipeline(cmd, bindPoint, pipeline); }

void Pipeline::DescriptorSets(VkCommandBuffer cmd, std::vector<VkDescriptorSet> sets, uint32_t setIndex)
{
    if (!sets.empty()) vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayoutHandle, setIndex, (uint32_t)sets.size(), sets.data(), 0, nullptr);
}

void Pipeline::PushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
{
    if (data && size > 0) vkCmdPushConstants(cmd, pipelineLayoutHandle, stageFlags, offset, size, data);
}
}