#include "editor.h"
#include "project/project.h"
#include "graphics/renderer.h"
#include "file_dialog.h"

void Editor::OnProjectOpened(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path / ".ballistic" / "editor");
    
    activeIniPath = (path / ".ballistic" / "editor" / "editor.ini").string();
    ImGui::GetIO().IniFilename = activeIniPath.c_str();
    if (std::filesystem::exists(activeIniPath)) {
        ImGui::LoadIniSettingsFromDisk(activeIniPath.c_str());
    } else {
        // ImGui::LoadIniSettingsFromMemory(DEFAULT_EDITOR_INI, strlen(DEFAULT_EDITOR_INI));
    }

    renderGraphContext = ImNodes::EditorContextCreate();
    ImNodes::EditorContextSet(renderGraphContext);
    std::string nodesPath = (path / ".ballistic/editor/nodes_rendergraph.ini").string();
    if (std::filesystem::exists(nodesPath)) {
        ImNodes::LoadCurrentEditorStateFromIniFile(nodesPath.c_str());
    } else  {

    }
}

void Editor::OnProjectClosed(const std::filesystem::path& path)
{
    if (!activeIniPath.empty()) {
        ImGui::SaveIniSettingsToDisk(activeIniPath.c_str());
        ImGui::GetIO().IniFilename = nullptr;
        activeIniPath.clear();
    }

    if (renderGraphContext) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::SaveCurrentEditorStateToIniFile((path / ".ballistic" / "editor" / "nodes_rendergraph.ini").string().c_str());
        ImNodes::EditorContextFree(renderGraphContext);
        renderGraphContext = nullptr;
    }
}

void Editor::Save(const std::filesystem::path& path)
{
    if (!activeIniPath.empty()) {
        ImGui::SaveIniSettingsToDisk(activeIniPath.c_str());
    }

    if (renderGraphContext) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::SaveCurrentEditorStateToIniFile((path / ".ballistic" / "editor" / "nodes_rendergraph.ini").string().c_str());
    }
}

void Editor::Update(Project& project)
{
    if (!project.IsOpen()) return;

    autosaveTimer += ImGui::GetIO().DeltaTime;
    if (autosaveEnabled && autosaveTimer >= autosaveInterval) {
        autosaveTimer = 0.0f;
        project.Save();
        Save(project.path);
        LOG_DEBUG("Autosaved project");
    }
}

void Editor::Draw(Project& project, Renderer& renderer, bool& pendingCloseProject, VkDescriptorSet finalTextureID)
{
    DrawDockSpace();

    DrawViewport(renderer, finalTextureID);
    DrawProjectPanel(project, renderer, pendingCloseProject);
    DrawRenderGraphPanel();
}

void Editor::DrawDockSpace()
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
}

void Editor::DrawViewport(Renderer& renderer, VkDescriptorSet finalTextureID)
{
    if (ImGui::Begin("Viewport")) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x < 1.0f) size.x = 1.0f;
        if (size.y < 1.0f) size.y = 1.0f;
        uint32_t w = (uint32_t)size.x;
        uint32_t h = (uint32_t)size.y;

        if (w != pendingViewportW || h != pendingViewportH) {
            pendingViewportW = w;
            pendingViewportH = h;
            viewportSizeChanged = true;
        }

        if (viewportSizeChanged && !ImGui::IsAnyItemActive()) {
            if (w > 0 && h > 0 && (w != lastViewportW || h != lastViewportH)) {
                lastViewportW = w;
                lastViewportH = h;
                renderer.RequestSceneResize(w, h);
            }
            viewportSizeChanged = false;
        }

        ImGui::Image((ImTextureID)finalTextureID, size);
    }
    ImGui::End();
}

void Editor::DrawProjectPanel(Project& project, Renderer& renderer, bool& pendingCloseProject)
{
    if (ImGui::Begin("Project")) {
        ImGui::TextDisabled("%s", project.path.string().c_str());
        ImGui::Separator();
        if (ImGui::Button(ICON_FA_HOUSE " Project Manager")) {
            project.Save();
            Save(project.path);
            pendingCloseProject = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save")) {
            project.Save();
            Save(project.path);
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

                if (renderer.RecreateImage(img)) {
                    project.images.push_back(img);
                    addImageNameBuffer[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!canAdd) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                addImageNameBuffer[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

    }
    ImGui::End();
}

void Editor::DrawRenderGraphPanel()
{
    if (ImGui::Begin("Render Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImNodes::EditorContextSet(renderGraphContext);
        ImNodes::BeginNodeEditor();

        ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = nullptr;
        ImNodes::GetStyle().Flags = ImNodesStyleFlags_NodeOutline | ImNodesStyleFlags_GridLines;

        ImNodesIO& io = ImNodes::GetIO();
        io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

        ImNodes::BeginNode(1);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("GBufferPass");
            ImNodes::EndNodeTitleBar();
            ImNodes::BeginOutputAttribute(2);
            ImGui::Text("GBuffer_Albedo");
            ImNodes::EndOutputAttribute();
            ImNodes::BeginOutputAttribute(3);
            ImGui::Text("GBuffer_Depth");
            ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        ImNodes::BeginNode(4);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("LightingPass");
            ImNodes::EndNodeTitleBar();
            ImNodes::BeginInputAttribute(5);
            ImGui::Text("GBuffer_Albedo");
            ImNodes::EndInputAttribute();
            ImNodes::BeginInputAttribute(6);
            ImGui::Text("GBuffer_Depth");
            ImNodes::EndInputAttribute();
            ImNodes::BeginOutputAttribute(7);
            ImGui::Text("HdrLight");
            ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        ImNodes::Link(1, 2, 5);
        ImNodes::Link(2, 3, 6);

        ImNodes::MiniMap();
        // ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();
    }
    ImGui::End();
}
