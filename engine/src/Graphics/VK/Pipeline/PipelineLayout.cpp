#include "PipelineLayout.h"

bool PipelineLayout::Create(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);

    Destroy();
    deviceHandle = device;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (!descriptorSetLayouts.empty()) {
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    } else {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }

    if (!pushConstants.empty()) {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    } else {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan pipeline layout");
        return false;
    }

    LOG_DEBUG("Pipeline Layout created");
    return true;
}

void PipelineLayout::Destroy()
{
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(deviceHandle, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Pipeline Layout destroyed");
    }
}
