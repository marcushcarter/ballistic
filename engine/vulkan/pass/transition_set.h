#pragma once
#include <vector>
#include <image/image_2d.h>
#include <buffer/buffer.h>

namespace Ballistic::Vulkan
{
struct TransitionSet
{
    struct ImageTransition {
        Image2D* image;
        VkImageLayout layout;
        VkPipelineStageFlags stage;
        VkAccessFlags access;
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    };
    
    struct BufferTransition {
        Buffer* buffer;
        VkPipelineStageFlags stage;
        VkAccessFlags access;
    };

    std::vector<ImageTransition> images;
    std::vector<BufferTransition> buffers;

    void AddImage(Image2D* image, VkImageLayout layout, VkPipelineStageFlags stage, VkAccessFlags access, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT);
    void AddBuffer(Buffer* buffer, VkPipelineStageFlags stage, VkAccessFlags access);
    void Clear();

    void Transition(VkCommandBuffer cmd);
};
}