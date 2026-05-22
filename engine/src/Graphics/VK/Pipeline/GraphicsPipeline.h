#pragma once
#include "pch.h"
#include "Pipeline.h"

inline VkVertexInputBindingDescription VertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX) {
    VkVertexInputBindingDescription b{};
    b.binding = binding;
    b.inputRate = rate;
    b.stride = stride;
    return b;
}

inline VkVertexInputAttributeDescription VertexAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset = 0) {
    VkVertexInputAttributeDescription a{};
    a.location = location;
    a.binding = binding;
    a.format = format;
    a.offset = offset;
    return a;
}

struct GraphicsPipelineDesc {
    // void* pNext = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    bool depthTest = false;
    bool depthWrite = false;
    VkCompareOp depthOp = VK_COMPARE_OP_LESS;
    uint32_t numColorAttachments = 1;
    float lineWidth = 1.0f;

    // VkRenderPass renderPass = VK_NULL_HANDLE;
    // uint32_t subpass = 0;

    // std::vector<VkFormat> colorAttachmentFormats;
    // VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    // VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
};

struct GraphicsPipeline : Pipeline
{
    GraphicsPipeline() : Pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS) {}

    bool Create(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCache pipelineCache, const GraphicsPipelineDesc& desc);
};