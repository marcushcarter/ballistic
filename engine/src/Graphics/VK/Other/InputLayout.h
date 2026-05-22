#pragma once
#include "pch.h"

struct InputLayout
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    void AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX) {
        VkVertexInputBindingDescription b{};
        b.binding = binding;
        b.inputRate = rate;
        b.stride = stride;
        bindings.push_back(b);
    }

    void AddAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
        VkVertexInputAttributeDescription a{};
        a.location = location;
        a.binding = binding;
        a.format = format;
        a.offset = offset;
        attributes.push_back(a);
    }

    void Clear() {
        bindings.clear();
        attributes.clear();
    }
};
