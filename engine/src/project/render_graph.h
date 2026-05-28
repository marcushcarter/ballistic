#pragma once
#include "pch.h"

enum class RGImageSizeMode { ViewportRelative, Fixed };

struct RGImage {
    uint64_t id = 0;
    std::string name;
    VkFormat format = VK_FORMAT_UNDEFINED;
    RGImageSizeMode sizeMode = RGImageSizeMode::ViewportRelative;
    float relativeWidth = 1.0f;
    float relativeHeight = 1.0f;
    uint32_t fixedWidth = 0;
    uint32_t fixedHeight = 0;
    bool usageAttachment = true;
    bool usageSampled = true;
    bool usageStorage = false;
};

// struct RGPass {
//     uint64_t id = 0;
//     std::string name;
//     bool enabled = true;
//     std::vector<uint64_t> reads;
//     std::vector<uint64_t> writes;
// };

// struct RenderGraph {
//     std::vector<uint64_t> passes;
// };

// struct CompiledGraph {
//     std::vector<RGPass*> sortedPasses;
// };