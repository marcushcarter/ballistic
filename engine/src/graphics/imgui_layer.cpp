#include "imgui_layer.h"
#include "renderer.h"

bool ImGuiLayer::Create(Renderer& renderer, GLFWwindow* window)
{    
    BE_ASSERT(descriptorPool.Create(renderer.device.Get(), {
        .samplers = 10,
        .combinedImageSamplers = 10,
        .sampledImages = 10,        
        .afterBind = false,
        .debugName = "ImGuiDescriptorPool"
    }));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &renderer.swapchain.format;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = renderer.instance.Get();
    initInfo.PhysicalDevice = renderer.physicalDevice.Get();
    initInfo.Device = renderer.device.Get();
    initInfo.QueueFamily = renderer.graphicsQueue.familyIndex;
    initInfo.Queue = renderer.graphicsQueue.Get();
    initInfo.DescriptorPool = descriptorPool.Get();
    initInfo.MinImageCount = renderer.frameCount;
    initInfo.ImageCount = renderer.frameCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;
    initInfo.CheckVkResultFn = [](VkResult err){ if(err) LOG_ERROR(" "); };

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_Init(&initInfo);

    ImNodes::CreateContext();
    ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = nullptr;
    ImNodes::GetStyle().Flags = ImNodesStyleFlags_NodeOutline | ImNodesStyleFlags_GridLines;

    LOG_DEBUG("ImGui created");
    return true;
}

void ImGuiLayer::Destroy()
{   
    ImNodes::DestroyContext();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    descriptorPool.Destroy();
    LOG_DEBUG("ImGui destroyed");
}

void ImGuiLayer::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::Render()
{
    ImGui::Render();
}

void ImGuiLayer::RecordDraw(VkCommandBuffer cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}
