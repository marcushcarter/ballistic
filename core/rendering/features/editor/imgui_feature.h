#pragma once
#include <core/rendering/features/feature.h>
#include <drivers/vulkan/device_driver_vulkan.h>

namespace ballistic {

struct ImGuiFeature : Feature
{
    RenderGraph::Pass ui_pass;

    VkAttachmentLoadOp backbuffer_load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    const char* sampled_image = nullptr;

    Error create_resources() override;
    void build(RenderGraph& g) override;
};

}