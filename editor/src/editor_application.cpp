#include "editor_application.h"
#include "resources.h"
#include "file_dialog.h"

void EditorApplication::OnInit()
{
    window.SetEmbeddedIcon(IMG_ICON_COMP_PNG);
    window.SetTitlebarColor(0.15f, 0.15f, 0.15f);
    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
        renderer.RequestSceneResize(w, h);
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

    projectManager.Start(roamingRoot, logoLongTextureID, renderer.logoLongImage.extent);
    projectManager.Load();

    onProjectLoadFailed = [this]() {
        inProjectManager = true;
    };
    
    LOG_DEBUG("Editor initialized");
}

void EditorApplication::OnUpdate()
{
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
        DrawEditor();
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
    activeIniPath = (path / "editor.ini").string();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = activeIniPath.c_str();
    ImGui::LoadIniSettingsFromDisk(activeIniPath.c_str());
    
    LOG_INFO("Editing project: %s", path.string().c_str());
}

void EditorApplication::OnProjectClosed()
{
    if (!activeIniPath.empty()) {
        ImGui::SaveIniSettingsToDisk(activeIniPath.c_str());
        ImGui::GetIO().IniFilename = nullptr;
        activeIniPath.clear();
    }
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

void EditorApplication::DrawEditor()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr,
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground
    );
    ImGui::PopStyleVar();
    ImGui::DockSpace(ImGui::GetID("MainDockSpace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // ImGui::Begin("Test Panel");
    // ImGui::Text("Ballistic Engine");
    // ImGui::Separator();
    // ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    // ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    // ImGui::End();

    // ImGui::Begin("Viewport");
    // ImVec2 size = ImGui::GetContentRegionAvail();
    // ImGui::Image((ImTextureID)finalTextureID, size);
    // size = ImVec2((float)renderer.logoImage.extent.width, (float)renderer.logoImage.extent.height);
    // ImGui::Image((ImTextureID)logoTextureID, size);
    // size = ImVec2((float)renderer.logoLongImage.extent.width, (float)renderer.logoLongImage.extent.height);
    // ImGui::Image((ImTextureID)logoLongTextureID, size);
    // ImGui::End();

    // ImGui::Begin("Test");
    // if (ImGui::Button("Quit")) {}
    // if (ImGui::Button("Save and Quit")) {}
    // ImGui::End();

    if (ImGui::Begin("Project")) {
        ImGui::TextDisabled("%s", project.path.string().c_str());
        ImGui::Separator();
        if (ImGui::Button(ICON_FA_HOUSE " Project Manager")) {
            CloseProject();
            inProjectManager = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_PLUS " Add Image"))
            ImGui::OpenPopup("Add Image");

        ImGui::Spacing();

        for (auto& img : project.images) {
            ImGui::Text("%s", img.name.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("[%llu]", img.id);
        }

        if (ImGui::BeginPopupModal("Add Image", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("Name");
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##imgname", "GBuffer_Albedo", addImageNameBuffer, sizeof(addImageNameBuffer));

            ImGui::Text("Format");
            const char* formats[] = {
                "R8G8B8A8_UNORM",
                "R16G16B16A16_SFLOAT",
                "R32G32B32A32_SFLOAT",
                "D32_SFLOAT",
                "D24_UNORM_S8_UINT"
            };
            ImGui::SetNextItemWidth(300);
            ImGui::Combo("##format", &addImageFormatIndex, formats, IM_ARRAYSIZE(formats));

            ImGui::Text("Size Mode");
            const char* sizeModes[] = { "Viewport Relative", "Fixed" };
            ImGui::SetNextItemWidth(300);
            ImGui::Combo("##sizemode", &addImageSizeModeIndex, sizeModes, 2);

            if (addImageSizeModeIndex == 0) {
                ImGui::Text("Scale");
                ImGui::SetNextItemWidth(140);
                ImGui::InputFloat("W##relw", &addImageRelW, 0.1f, 0.0f, "%.2f");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                ImGui::InputFloat("H##relh", &addImageRelH, 0.1f, 0.0f, "%.2f");
            } else {
                ImGui::Text("Size");
                ImGui::SetNextItemWidth(140);
                ImGui::InputScalar("W##fixw", ImGuiDataType_U32, &addImageFixedW);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                ImGui::InputScalar("H##fixh", ImGuiDataType_U32, &addImageFixedH);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            static const VkFormat formatMap[] = {
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_FORMAT_R16G16B16A16_SFLOAT,
                VK_FORMAT_R32G32B32A32_SFLOAT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT
            };

            bool nameEmpty = addImageNameBuffer[0] == '\0';
            bool nameTaken = false;
            for (auto& img : project.images)
                if (img.name == addImageNameBuffer) { nameTaken = true; break; }

            if (nameEmpty || nameTaken) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                if (nameEmpty)  ImGui::TextUnformatted("Name cannot be empty.");
                if (nameTaken)  ImGui::TextUnformatted("Name already exists.");
                ImGui::PopStyleColor();
            }

            bool canAdd = !nameEmpty && !nameTaken;
            if (!canAdd) ImGui::BeginDisabled();
            if (ImGui::Button("Add", ImVec2(120, 0))) {
                RGImage img;
                img.id = GenerateID();
                img.name = addImageNameBuffer;
                img.format = formatMap[addImageFormatIndex];
                img.sizeMode = addImageSizeModeIndex == 0 ? RGImageSizeMode::ViewportRelative : RGImageSizeMode::Fixed;
                img.relativeWidth = addImageRelW;
                img.relativeHeight = addImageRelH;
                img.fixedWidth = addImageFixedW;
                img.fixedHeight = addImageFixedH;

                renderer.RecreateImage(img);
                project.images.push_back(img);

                addImageNameBuffer[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            if (!canAdd) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                addImageNameBuffer[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }
}
