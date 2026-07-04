#include <core/rendering/renderer.h>
#include <core/log/error_macros.h>
#include <iostream>

namespace ballistic {

Error Renderer::create(drivers::DeviceDriverVulkan& r_device_driver)
{
    using enum Error;

    device_driver = &r_device_driver;
    frame_count = device_driver->frame_count;
    current_frame = 0;

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

    graph.create(r_device_driver, frame_count);

    return Ok;
}

void Renderer::destroy()
{
    graph.destroy();

    device_driver->image_free(final_image);

    for (uint32_t i = 0; i < frame_count; i++) {
        device_driver->fence_free(in_flight_fences[i]);
        device_driver->semaphore_free(image_available_semaphores[i]);
        device_driver->command_pool_free(command_pools[i]);
    }
}

Error Renderer::set_size(uint32_t p_width, uint32_t p_height)
{
    using enum Error;

    if (p_width == 0 || p_height == 0) return Ok;
    if (p_width == width && p_height == height) return Ok;
    width = p_width;
    height = p_height;
    
    device_driver->device_wait_idle();

    device_driver->image_free(final_image);
    drivers::DeviceDriverVulkan::ImageCreateInfo image_ci{};
    image_ci.name = "final_image";
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.layers = 1;
    final_image = device_driver->image_create_dedicated(image_ci, { width, height, 1 });
    // final_image.state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    // final_image.state.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    // final_image.state.access = 0;

    graph.set_size(p_width, p_height);

    return Ok;
}

Error Renderer::begin_frame()
{
    using enum Error;
    
    auto& sc = device_driver->swapchain;

    Error err = device_driver->fence_wait(in_flight_fences[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = device_driver->fence_reset(in_flight_fences[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = device_driver->swapchain_acquire_next_image(image_available_semaphores[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = device_driver->command_pool_reset(command_pools[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = device_driver->command_buffer_begin(command_buffers[current_frame], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    cmd = command_buffers[current_frame];

    auto& bb = sc.images[sc.image_index];
    bb.state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    bb.state.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    bb.state.access = 0;

    graph.begin();
    graph.import_image("final_image", &final_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    graph.import_image("backbuffer", &bb, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    return Ok;
}

Error Renderer::end_frame()
{
    using enum Error;
    auto& sc = device_driver->swapchain;

    graph.compile();
    graph.execute(cmd);
    
    Error err = device_driver->command_buffer_end(command_buffers[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &sc.present_semaphores[sc.image_index];

    VkQueue graphics_queue = device_driver->queue_families[device_driver->context_driver->graphics_queue_family][0].queue;
    VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);
    BALLISTIC_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, err, "Failed to submit Vulkan queue");

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &sc.present_semaphores[sc.image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &sc.swapchain;
    present_info.pImageIndices = &sc.image_index;

    VkQueue present_queue = device_driver->queue_families[device_driver->context_driver->present_queue_family][0].queue;
    result = vkQueuePresentKHR(present_queue, &present_info);
    BALLISTIC_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, err, "Failed to present Vulkan queue");

    current_frame = (current_frame + 1) % frame_count;
    frame_number++;

    return Ok;
}

}