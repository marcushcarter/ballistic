#pragma once
#include "pch.h"

enum class ImageSizeMode { ViewportRelative, Fixed };

struct ResourceImageDesc
{
    uint64_t id = 0;
    std::string name;
    VkFormat format = VK_FORMAT_UNDEFINED;

    ImageSizeMode sizeMode = ImageSizeMode::ViewportRelative;
    float relativeWidth = 1.0f;
    float relativeHeight = 1.0f;
    uint32_t fixedWidth = 0;
    uint32_t fixedHeight = 0;

    bool usageAttachment = true;
    bool usageSampled = true;
    bool usageStorage = false;
};

struct RGPass
{
    uint64_t id = 0;
    std::string name;
    std::string engineHook;

    bool enabled = true;
    std::vector<uint64_t> reads;
    std::vector<uint64_t> writes;
};

struct RenderGraph
{
    std::vector<uint64_t> passOrder;
};
