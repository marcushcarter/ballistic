#include "imgui_layer.h"
#include "renderer.h"

bool ImGuiLayer::Create(Renderer& renderer, GLFWwindow* window)
{
    // TODO: move from Renderer::CreateImGui
    (void)renderer;
    (void)window;
    return false;
}

void ImGuiLayer::Destroy()
{
    // TODO: move from Renderer::DestroyImGui
}

void ImGuiLayer::NewFrame()
{
    // TODO: ImGui_ImplVulkan_NewFrame + ImGui_ImplGlfw_NewFrame + ImGui::NewFrame
}

void ImGuiLayer::Render(VkCommandBuffer cmd)
{
    // TODO: ImGui::Render + ImGui_ImplVulkan_RenderDrawData
    (void)cmd;
}
