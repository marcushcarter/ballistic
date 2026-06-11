#include <pipeline/graphics_pipeline.h>
#include <misc/utils.h>

namespace Ballistic::Vulkan
{
bool GraphicsPipeline::Create(VkDevice device, const GraphicsPipelineDesc& d)
{
    Destroy();
    debugName = d.debugName;
    deviceHandle = device;
    pipelineLayoutHandle = d.layout;

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = (uint32_t)d.bindings.size();
    vertexInput.pVertexBindingDescriptions = d.bindings.data();
    vertexInput.vertexAttributeDescriptionCount = (uint32_t)d.attributes.size();
    vertexInput.pVertexAttributeDescriptions = d.attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = d.topology;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster{};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.polygonMode = d.polygonMode;
    raster.cullMode = d.cullMode;
    raster.frontFace = d.frontFace;
    raster.lineWidth = d.lineWidth;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth{};
    depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth.depthTestEnable = d.depthTest;
    depth.depthWriteEnable = d.depthWrite;
    depth.depthCompareOp = d.depthOp;

    std::vector<VkPipelineColorBlendAttachmentState> colors(d.numColorAttachments);
    for (auto& c : colors) {
        c.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        c.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo blend{};
    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.attachmentCount = (uint32_t)colors.size();
    blend.pAttachments = colors.data();

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic{};
    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.dynamicStateCount = (uint32_t)dynamicStates.size();
    dynamic.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = d.pNext;
    createInfo.stageCount = (uint32_t)d.shaderStages.size();
    createInfo.pStages = d.shaderStages.data();
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &raster;
    createInfo.pMultisampleState = &ms;
    createInfo.pDepthStencilState = &depth;
    createInfo.pColorBlendState = &blend;
    createInfo.pDynamicState = &dynamic;
    createInfo.layout = d.layout;
    createInfo.renderPass = d.renderPass;
    createInfo.subpass = d.subpass;

    if (vkCreateGraphicsPipelines(device, d.cache, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        // LOG_ERROR("Graphics Pipeline create failed: %s - vkCreateGraphicsPipelines", debugName ? debugName : "Unnamed");
        return false;
    }

    SetObjectName(device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)pipeline, d.debugName);
    // LOG_DEBUG("Graphics Pipeline created: %s", debugName ? debugName : "Unnamed");
    return true;
}
}