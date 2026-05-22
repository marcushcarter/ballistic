#pragma once
#include "pch.h"
#include "VK.h"

inline void VKViewport(VkCommandBuffer cmd, float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) {
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width  = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

inline void VKScissor(VkCommandBuffer cmd, float x, float y, float width, float height) {
    VkRect2D scissor{};
    scissor.offset = { static_cast<int32_t>(x), static_cast<int32_t>(y) };
    scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

inline void VKViewportScissor(VkCommandBuffer cmd, float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) {
    VKViewport(cmd, x, y, width, height, minDepth, maxDepth);
    VKScissor(cmd, x, y, width, height);
}

inline VkClearValue VKClearColor(float r = 0, float g = 0, float b = 0, float a = 1) {
    VkClearValue v{};
    v.color = { r, g, b, a };
    return v;
}

inline VkClearValue VKClearDepth(float depth = 1, uint32_t stencil = 0) {
    VkClearValue v{};
    v.depthStencil = { depth, stencil };
    return v;    
}