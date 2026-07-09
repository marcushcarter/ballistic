#include <core/dev_tools/dev_tools.h>
#include <core/rendering/renderer.h>
#include <imgui.h>

namespace ballistic {

Error DevTools::create(Renderer& r_renderer, drivers::DeviceDriverVulkan& r_device_driver)
{
    renderer = &r_renderer;
    device_driver = &r_device_driver;
    texture_cache.create(r_device_driver);
    return Error::Ok;
}

void DevTools::destroy()
{
    texture_cache.destroy();
}

void DevTools::begin_frame()
{
    texture_cache.begin_frame(renderer->frame_number, renderer->frame_count, renderer->resize_epoch);
}

void DevTools::end_frame()
{
    texture_cache.collect(renderer->frame_number);
}

void DevTools::draw_menu()
{
    if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem("RenderBuffer XRay", nullptr, &renderbuffer_xray.open);
        ImGui::MenuItem("Debug Console", nullptr, &debug_console.open);
        ImGui::MenuItem("GPU Profiler", nullptr, &gpu_profiler.open);
        ImGui::Separator();

        if (ImGui::MenuItem("Close All")) {
            renderbuffer_xray.open = false;
            debug_console.open = false;
            gpu_profiler.open = false;
        }

        ImGui::EndMenu();
    }
}

void DevTools::draw_tools(bool editor)
{
    if (!editor) renderbuffer_xray.draw(renderer->graph, texture_cache);
    debug_console.draw();
    gpu_profiler.draw(renderer->graph);
}

}