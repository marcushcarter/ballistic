#include "pipeline_layout.h"
#include "graphics/vk/misc/utils.h"

bool PipelineLayout::Create(VkDevice device, const PipelineLayoutDesc& desc)
{
    VK_CHECK_HANDLE(device, VkDevice);

    Destroy();
    debugName = desc.debugName;
    deviceHandle = device;

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<uint32_t>(desc.setLayouts.size());
    createInfo.pSetLayouts = desc.setLayouts.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(desc.pushConstants.size());
    createInfo.pPushConstantRanges = desc.pushConstants.data();

    if (vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("Pipeline Layout create failed: %s - vkCreatePipelineLayout", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)pipelineLayout, debugName);
    LOG_DEBUG("Pipeline Layout created: %s", debugName ? debugName : "Unnamed");
    return true;
}

void PipelineLayout::Destroy()
{
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(deviceHandle, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
        LOG_DEBUG("Pipeline Layout destroyed: %s", debugName ? debugName : "Unnamed");
    }
}
