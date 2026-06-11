#pragma once

namespace Ballistic::Vulkan
{
inline void Viewport(VkCommandBuffer cmd, float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
{
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width  = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

inline void Scissor(VkCommandBuffer cmd, float x, float y, float width, float height)
{
    VkRect2D scissor{};
    scissor.offset = { static_cast<int32_t>(x), static_cast<int32_t>(y) };
    scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

inline void ViewportScissor(VkCommandBuffer cmd, float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
{
    Viewport(cmd, x, y, width, height, minDepth, maxDepth);
    Scissor(cmd, x, y, width, height);
}

inline void SetObjectName(VkDevice device, VkObjectType type, uint64_t handle, const char* name)
{
    if (!name) return;
    auto fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (!fn) return;
    VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    info.objectType = type;
    info.objectHandle = handle;
    info.pObjectName  = name;
    fn(device, &info);
}
}