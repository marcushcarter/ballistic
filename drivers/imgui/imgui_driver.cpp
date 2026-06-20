#include <drivers/imgui/imgui_driver.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <iostream>

namespace ballistic::drivers {
    
void ImGuiDriver::create(const ImGuiDriverCreateInfo& p_info)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    // create descriptor pool

    VkPipelineRenderingCreateInfo pipeline_rendering_info{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &p_info.color_format;
    
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = p_info.instance;
    init_info.PhysicalDevice = p_info.physical_device;
    init_info.Device = p_info.device;
    init_info.QueueFamily = p_info.queue_family;
    init_info.Queue = p_info.queue;
    init_info.DescriptorPool = p_info.descriptor_pool;
    init_info.MinImageCount = p_info.image_count;
    init_info.ImageCount = p_info.image_count;
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipeline_rendering_info;
    init_info.CheckVkResultFn = [](VkResult err){ if(err) std::cerr << " "; };

    ImGui_ImplWin32_Init(p_info.hwnd);
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiDriver::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    // descriptorPool.Destroy();
}

void ImGuiDriver::new_frame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDriver::render()
{
    ImGui::Render();
}

void ImGuiDriver::record_commands(VkCommandBuffer p_cmd)
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, p_cmd);
    }
}

}