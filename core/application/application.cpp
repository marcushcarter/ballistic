#include <core/application/application.h>
#include <core/error/error_macros.h>
#include <core/version.h>
#include <windows.h>
#include <chrono>
#include <iostream>

namespace ballistic {

Error Application::create(const ApplicationCreateInfo& p_info)
{
    using enum Error;
    Error err;

    std::cout << BALLISTIC_VERSION_NAME << " v" << BALLISTIC_VERSION_NUMBER << ".stable.official - https://ballisticgames.ca\n";
    
    create_info = p_info;

    err = window.create(p_info.window_title, p_info.width, p_info.height);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = context_driver.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be called for other backends
    err = context_driver.surface_create(window.hwnd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    // can be manually selected
    err = context_driver.physical_device_select();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    device_driver.context_driver = &context_driver;
    err = device_driver.initialize(context_driver.optimal_device_index, 1);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    renderer.device_driver = &device_driver;
    err = renderer.create(3);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    drivers::ImGuiDriverCreateInfo imgui_ci{};
    imgui_ci.hwnd = window.hwnd;
    imgui_ci.instance = context_driver.instance;
    imgui_ci.physical_device = device_driver.physical_device;
    imgui_ci.device = device_driver.device;
    imgui_ci.queue_family = context_driver.graphics_queue_family;
    imgui_ci.queue = device_driver.queue_families[context_driver.graphics_queue_family][0].queue;
    imgui_ci.color_format = device_driver.swapchain.format;
    imgui_ci.image_count = 3;

    err = imgui.create(imgui_ci);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

void Application::destroy()
{
    device_driver.device_wait_idle();

    imgui.destroy();
    renderer.destroy();
    device_driver.shutdown();
    context_driver.shutdown();
    window.destroy();
}

int Application::run()
{
    using enum Error;
    Error err;

    err = on_init();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, static_cast<int>(err));

    auto lastTime = std::chrono::steady_clock::now();

    while (!window.should_close()) {
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration<double>(now - lastTime).count();
        lastTime = now;

        window.poll_events();
        context_driver.surface_set_size(window.width, window.height);
        device_driver.update_swapchain();
        
        err = renderer.set_size(window.width, window.height);
        BALLISTIC_ERR_FAIL_COND_V(err != Ok, static_cast<int>(err));
        
        imgui.new_frame();
        on_update((float)delta);
        imgui.render();

        renderer.begin_frame();

        VkRenderingAttachmentInfo color_attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color_attachment.imageView = device_driver.swapchain.image_views[device_driver.swapchain.image_index];
        color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderingInfo rendering_info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        rendering_info.renderArea = { { 0, 0 }, { context_driver.surface.width, context_driver.surface.height } };
        rendering_info.layerCount = 1;
        rendering_info.colorAttachmentCount = 1;
        rendering_info.pColorAttachments = &color_attachment;

        vkCmdBeginRendering(renderer.cmd, &rendering_info);
        imgui.record_commands(renderer.cmd);
        vkCmdEndRendering(renderer.cmd);

        renderer.end_frame();
    }

    on_shutdown();
    destroy();
    return 0;
}

}