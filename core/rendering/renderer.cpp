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

    drivers::DeviceDriverVulkan::RenderPassCreateInfo swapchain_render_pass_ci{};
    swapchain_render_pass_ci.attachments.push_back({
        .format = device_driver->swapchain.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .is_depth = false,
    });
    swapchain_render_pass_ci.name = "imgui_present_render_pass";
    swapchain_render_pass = device_driver->render_pass_create(swapchain_render_pass_ci);

    drivers::DeviceDriverVulkan::BufferCreateInfo buffer_ci{};
    buffer_ci.size = sizeof(float) * 4;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.memory = drivers::DeviceDriverVulkan::BufferCreateInfo::Memory::HostVisible;
    buffer_ci.name = "float4";
    test_buffer = device_driver->buffer_create(buffer_ci);

    float color[4] = { 1.0, 0.0, 1.0, 1.0 };
    device_driver->buffer_update(test_buffer, &color, sizeof(float)*4);

    for (uint32_t i = 0; i < frame_count; i++) {
        in_flight_fences[i] = device_driver->fence_create(true);
        image_available_semaphores[i] = device_driver->semaphore_create();
        command_pools[i] = device_driver->command_pool_create(graphics_family);
        command_buffers[i] = device_driver->command_buffer_create(command_pools[i]);
    }

    graph.create(r_device_driver, frame_count);

    set_size(1, 1);

    return Ok;
}

void Renderer::destroy()
{
    graph.destroy();

    device_driver->image_free(final_image);
    device_driver->image_free(depth_image);
    device_driver->image_free(image_2);

    for (uint32_t i = 0; i < frame_count; i++) {
        device_driver->fence_free(in_flight_fences[i]);
        device_driver->semaphore_free(image_available_semaphores[i]);
        device_driver->command_pool_free(command_pools[i]);
    }

    device_driver->render_pass_free(swapchain_render_pass);
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

    device_driver->image_free(depth_image);
    drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
    depth_ci.format = VK_FORMAT_D32_SFLOAT;
    depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_ci.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
    depth_ci.width_scale  = 1.0f;
    depth_ci.height_scale = 1.0f;
    depth_image = device_driver->image_create_dedicated(depth_ci, { width, height, 1 });

    device_driver->image_free(image_2);    
    drivers::DeviceDriverVulkan::ImageCreateInfo image_ci2{};
    image_ci2.name = "test_color";
    image_ci2.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci2.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci2.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    image_ci2.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative;
    image_ci2.width_scale  = 1.0f;
    image_ci2.height_scale = 1.0f;
    image_2 = device_driver->image_create_dedicated(image_ci2, { width, height, 1 });

    Error err = graph.set_size(p_width, p_height);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

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

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, device_driver->bindless_heap.pipeline_layout, 0, 1, &device_driver->bindless_heap.set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, device_driver->bindless_heap.pipeline_layout, 0, 1, &device_driver->bindless_heap.set, 0, nullptr);

    graph.begin(current_frame);
    graph.import_image("final_image", &final_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    graph.import_image("imp_depth", &depth_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    graph.import_image("imp_color", &image_2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    graph.import_image("backbuffer", &sc.images[sc.image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);
    graph.import_buffer("test_buffer", &test_buffer, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

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