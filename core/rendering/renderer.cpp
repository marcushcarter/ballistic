#include <core/rendering/renderer.h>
#include <core/error/error_macros.h>

namespace ballistic {

Error Renderer::create(uint32_t p_frame_count)
{
    using enum Error;
    Error err;

    frame_count = p_frame_count;
    current_frame = 0;

    swapchain = device_driver->swapchain_create(&device_driver->context_driver->surface);
    err = device_driver->swapchain_resize(swapchain, frame_count);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    in_flight_fences.resize(frame_count);
    image_available_semaphores.resize(frame_count);
    command_pools.resize(frame_count);
    command_buffers.resize(frame_count);

    uint32_t graphics_family = device_driver->context_driver->graphics_queue_family;

    for (uint32_t i = 0; i < frame_count; i++) {
        in_flight_fences[i] = device_driver->fence_create(true);

        image_available_semaphores[i] = device_driver->semaphore_create();

        command_pools[i] = device_driver->command_pool_create(graphics_family);

        command_buffers[i] = device_driver->command_buffer_create(command_pools[i]);
    }

    return Ok;
}

void Renderer::destroy()
{

}

static void command_image_barrier(VkCommandBuffer p_cmd, VkImage p_image, VkImageLayout p_old_layout, VkImageLayout p_new_layout, VkPipelineStageFlags2 p_src_stage, VkPipelineStageFlags2 p_dst_stage, VkAccessFlags2 p_src_access, VkAccessFlags2 p_dst_access)
{
    VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier.srcStageMask = p_src_stage;
    barrier.srcAccessMask = p_src_access;
    barrier.dstStageMask = p_dst_stage;
    barrier.dstAccessMask = p_dst_access;
    barrier.oldLayout = p_old_layout;
    barrier.newLayout = p_new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = p_image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dep_info{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(p_cmd, &dep_info);
}

Error Renderer::begin_frame()
{
    using enum Error;

    device_driver->fence_wait(in_flight_fences[current_frame]);
    device_driver->fence_reset(in_flight_fences[current_frame]);

    vkAcquireNextImageKHR(device_driver->device, swapchain.swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &swapchain.image_index);

    Error err = device_driver->command_pool_reset(command_pools[current_frame]);
    device_driver->command_buffer_begin(command_buffers[current_frame], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    (void)err;

    cmd = command_buffers[current_frame];

    command_image_barrier(cmd, swapchain.images[swapchain.image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    return Ok;
}

Error Renderer::end_frame()
{
    using enum Error;

    command_image_barrier(cmd, swapchain.images[swapchain.image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0);

    device_driver->command_buffer_end(command_buffers[current_frame]);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &swapchain.present_semaphores[swapchain.image_index];

    VkQueue graphics_queue = device_driver->queue_families[device_driver->context_driver->graphics_queue_family][0].queue;
    VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &swapchain.present_semaphores[swapchain.image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain.swapchain;
    present_info.pImageIndices = &swapchain.image_index;

    VkQueue present_queue = device_driver->queue_families[device_driver->context_driver->present_queue_family][0].queue;
    result = vkQueuePresentKHR(present_queue, &present_info);
    (void)result;

    current_frame = (current_frame + 1) % frame_count;

    return Ok;
}

}