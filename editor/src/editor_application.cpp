#include "editor_application.h"
#include "resources.h"

void EditorApplication::OnInit()
{
    window.SetEmbeddedIcon(IMG_ICON_COMP_PNG);
    window.SetTitlebarColor(0.15f, 0.15f, 0.15f);
    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
    };
    
    SetupAppData();

    renderer.CreateImGui(window.glfwWindow);
    renderer.onSwapchainPass = [this](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    };

    finalTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        renderer.finalImage.GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    logoTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        renderer.logoImage.GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    logoLongTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        renderer.logoLongImage.GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    renderer.onViewportResized = [this]() {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)finalTextureID);
        finalTextureID = ImGui_ImplVulkan_AddTexture(
            renderer.linearSampler.Get(),
            renderer.finalImage.GetView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    };

    projectManager.Start(roamingRoot, logoLongTextureID, renderer.logoLongImage.extent);
    projectManager.Load();

    onProjectLoadFailed = [this]() {
        inProjectManager = true;
    };
    
    LOG_DEBUG("Editor initialized");
}

void EditorApplication::OnUpdate()
{
    if (pendingCloseProject) {
        CloseProject();
        pendingCloseProject = false;
    }

    editor.Update(project);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (inProjectManager) {
        window.SetTitle("Ballistic Engine - Project Manager");
        std::filesystem::path requested = projectManager.Draw();
        if (!requested.empty()) {
            inProjectManager = false;
            OpenProject(requested);
        }
    } else {
        window.SetTitle(("Ballistic Engine - " + project.name).c_str());
        editor.Draw(project, renderer, pendingCloseProject, finalTextureID);
    }

    ImGui::Render();
}

void EditorApplication::OnShutdown()
{
    renderer.DestroyImGui();
    LOG_DEBUG("Editor shutdown");
}

void EditorApplication::OnProjectOpened(const std::filesystem::path& path)
{
    editor.OnProjectOpened(path);
    LOG_INFO("Editing project: %s", path.string().c_str());
}

void EditorApplication::OnProjectClosed()
{
    editor.OnProjectClosed(project.path);
    inProjectManager = true;
}

void EditorApplication::SetupAppData()
{
    PWSTR rawRoaming = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &rawRoaming);
    roamingRoot = std::filesystem::path(rawRoaming) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawRoaming);
    std::filesystem::create_directories(roamingRoot);

    PWSTR rawLocal = nullptr;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &rawLocal);
    localRoot = std::filesystem::path(rawLocal) / "Ballistic" / "BallisticEngine";
    CoTaskMemFree(rawLocal);
    std::filesystem::create_directories(localRoot);

    LOG_DEBUG("Roaming AppData: %s", roamingRoot.string().c_str());
    LOG_DEBUG("Local AppData: %s", localRoot.string().c_str());
}
