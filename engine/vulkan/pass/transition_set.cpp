#include <pass/transition_set.h>

namespace Ballistic::Vulkan
{
void TransitionSet::AddImage(Image2D* image, VkImageLayout layout, VkPipelineStageFlags stage, VkAccessFlags access, VkImageAspectFlags aspect)
{
    images.push_back({ image, layout, stage, access, aspect });
}

void TransitionSet::AddBuffer(Buffer* buffer, VkPipelineStageFlags stage, VkAccessFlags access)
{
    buffers.push_back({ buffer, stage, access });
}

void TransitionSet::Clear()
{
    images.clear();
    buffers.clear();
}

void TransitionSet::Transition(VkCommandBuffer cmd)
{
    if (images.empty() && buffers.empty()) return;

    std::vector<VkImageMemoryBarrier> imageBarriers;
    std::vector<VkBufferMemoryBarrier> bufferBarriers;

    VkPipelineStageFlags srcStageMask = 0;
    VkPipelineStageFlags dstStageMask = 0;

    for (auto& t : images) {
        if (t.image->layout == t.layout) {
            dstStageMask |= t.stage;
            continue;
        }

        VkPipelineStageFlags srcStage;
        VkAccessFlags srcAccess;

        switch (t.image->layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                srcAccess = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                srcAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                srcAccess = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                srcAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                break;
            default:
                srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                srcAccess = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                break;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = t.image->layout;
        barrier.newLayout = t.layout;
        barrier.srcAccessMask = srcAccess;
        barrier.dstAccessMask = t.access;
        barrier.image = t.image->image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = t.aspect;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = t.image->layers;
        imageBarriers.push_back(barrier);

        srcStageMask |= srcStage;
        dstStageMask |= t.stage;

        t.image->layout = t.layout;
        t.image->stage = t.stage;
        t.image->access = t.access;
    }

    for (auto& b : buffers) {
        VkBufferMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = b.buffer->access;
        barrier.dstAccessMask = b.access;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = b.buffer->buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        bufferBarriers.push_back(barrier);

        srcStageMask |= b.buffer->stage ? b.buffer->stage : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageMask |= b.stage;

        b.buffer->stage = b.stage;
        b.buffer->access = b.access;
    }

    if (imageBarriers.empty() && bufferBarriers.empty()) return;

    vkCmdPipelineBarrier(cmd, srcStageMask ? srcStageMask : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dstStageMask, 0, 0, nullptr, static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(), static_cast<uint32_t>(imageBarriers.size()),  imageBarriers.data());
}
}