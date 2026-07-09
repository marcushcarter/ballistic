#include <core/rendering/renderer.h>
#include <core/log/error_macros.h>
#include <core/io/embedded_resource.h>
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

    set_size(1, 1);
    pending_width = width;
    pending_height = height;

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
    resize_epoch++;
    
    device_driver->device_wait_idle();

    device_driver->image_free(final_image);
    drivers::DeviceDriverVulkan::ImageCreateInfo image_ci{};
    image_ci.name = "final_image";
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.layers = 1;
    final_image = device_driver->image_create_dedicated(image_ci, { width, height, 1 });

    Error err = graph.set_size(p_width, p_height);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

void Renderer::request_size(uint32_t p_width, uint32_t p_height)
{
    pending_width = p_width;
    pending_height = p_height;
}

Error Renderer::apply_pending_size()
{
    if (pending_width == 0 || pending_height == 0) return Error::Ok;
    return set_size(pending_width, pending_height);
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

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, device_driver->bindless_heap.pipeline_layout, 0, 1, &device_driver->bindless_heap.set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, device_driver->bindless_heap.pipeline_layout, 0, 1, &device_driver->bindless_heap.set, 0, nullptr);

    graph.begin(current_frame);
    graph.import_image("final_image", &final_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    graph.import_image("backbuffer", &sc.images[sc.image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);

    return Ok;
}

void Renderer::compile() { graph.compile(); }

void Renderer::render_frame() { graph.execute(cmd); }

Error Renderer::end_frame()
{
    using enum Error;
    auto& sc = device_driver->swapchain;
    
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
    // result = vkQueuePresentKHR(present_queue, &present_info);
    // BALLISTIC_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, err, "Failed to present Vulkan queue");

    result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        device_driver->swapchain.surface->needs_resize = true;
    } else {
        BALLISTIC_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, Failed, "Failed to present Vulkan queue");
    }

    current_frame = (current_frame + 1) % frame_count;
    frame_number++;

    return Ok;
}

}