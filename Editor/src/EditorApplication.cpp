#include "EditorApplication.h"
#include "Resources.h"
#include "FileDialog.h"

void EditorApplication::OnInit()
{
    window.SetEmbeddedIcon(IMG_ICON_COMP_PNG);
    window.SetTitlebarColor(0.15f, 0.15f, 0.15f);

    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
    };
    
    SetupAppData();
    LoadProjects();

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
        DrawProjectManager();
    } else {
        DrawEditor();
    }

    ImGui::Render();
    
    if (openProjectRequested) {
        openProjectRequested = false;
        inProjectManager = false;
        OpenProject(pendingOpenPath);

        // if  (!projectLoadFailed && currentProjectIndex >= 0 && currentProjectIndex < (int)projects.size()) {
        //     auto now = std::chrono::system_clock::now();
        //     std::time_t t = std::chrono::system_clock::to_time_t(now);
        //     std::tm tm{};
        //     localtime_s(&tm, &t);
        //     char buf[32];
        //     strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        //     projects[currentProjectIndex].lastOpened = buf;
        //     SaveProjects();
        // }
    }
}

void EditorApplication::OnShutdown()
{
    renderer.DestroyImGui();
    LOG_DEBUG("Editor shutdown");
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

    LOG_INFO("Roaming AppData: %s", roamingRoot.string().c_str());
    LOG_INFO("Local AppData: %s", localRoot.string().c_str());
}

void EditorApplication::LoadProjects()
{
    projects.clear();
    std::filesystem::path cfgPath = roamingRoot / "projects.cfg";
    if (!std::filesystem::exists(cfgPath)) return;

    try {
        auto root = toml::parse_file(cfgPath.string());
        auto arr = root["projects"].as_array();
        if (!arr) return;

        arr->for_each([&](auto& el) {
            if (!el.is_table()) return;
            auto& t = *el.as_table();
            ProjectEntry p;
            p.path = t["path"].value_or<std::string>("");
            p.favorite = t["favorite"].value_or(false);
            p.lastOpened = t["last_opened"].value_or<std::string>("");
            if (p.path.empty()) return;

            std::filesystem::path blstPath = std::filesystem::path(p.path) / "project.blst";
            if (std::filesystem::exists(blstPath)) {
                try {
                    auto blst = toml::parse_file(blstPath.string());
                    p.name = blst["name"].value_or<std::string>(std::filesystem::path(p.path).filename().string());
                    p.engineVersion = blst["engine_version"].value_or<std::string>("");
                } catch (...) {
                    p.name = std::filesystem::path(p.path).filename().string();
                }
            } else {
                p.name = std::filesystem::path(p.path).filename().string();
            }

            projects.push_back(p);
        });

    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse projects.cfg: %s", e.what());
    }

    for (auto& p : projects)
        LOG_INFO("Project: %s | %s | %s | %s | %s", p.name.c_str(), p.path.c_str(), p.lastOpened.c_str(), p.engineVersion.c_str(), p.favorite ? "fav" : "");
}

void EditorApplication::SaveProjects()
{
    toml::table root;
    toml::array projectsArray;

    for (auto& p : projects) {
        toml::table entry;
        entry.insert("name",           p.name);
        entry.insert("path",           p.path);
        entry.insert("favorite",       p.favorite);
        entry.insert("last_opened",    p.lastOpened);
        entry.insert("engine_version", p.engineVersion);
        projectsArray.push_back(entry);
    }

    root.insert("projects", projectsArray);

    std::ofstream file(roamingRoot / "projects.cfg");
    file << root;
}

void EditorApplication::DrawProjectManager()
{
    window.SetTitle("Ballistic Engine - Project Manager");
    
    float rightPanelWidth = 150.0f;
    float bottomBarHeight = 20;
    float sortWidth = 200.0f;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("ProjectManager", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
    );
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    float logoH = 28.0f;
    float logoW = logoH * ((float)renderer.logoLongImage.extent.width / (float)renderer.logoLongImage.extent.height);
    ImGui::Image((ImTextureID)logoLongTextureID, ImVec2(logoW, logoH));
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));
    if (ImGui::Button(ICON_FA_WRENCH " Settings")) {}
    ImGui::PopStyleColor(3);
    ImGui::Spacing();
    
    bool openCreatePopup = false;
    float mainBoxHeight = ImGui::GetContentRegionAvail().y - bottomBarHeight;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::BeginChild("MainBox", ImVec2(0, mainBoxHeight), ImGuiChildFlags_Borders);
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);
    if (ImGui::Button(ICON_FA_PLUS " Create")) openCreatePopup = true;
    ImGui::SameLine();
    
    if (ImGui::Button(ICON_FA_FOLDER " Import")) {
        std::string picked = FileDialog("Import Project", nullptr, false, {
        { L"Ballistic Project", L"*.blst" },
        { L"All Files", L"*.*" }
    });

    if (!picked.empty()) {
        std::filesystem::path blstPath(picked);
        std::filesystem::path projectFolder = blstPath.parent_path();

        std::string name;
        std::string engineVersion;
        std::ifstream file(blstPath);
        std::string line;
        while (std::getline(file, line)) {
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            if (key == "name") name = val;
            else if (key == "engine_version") engineVersion = val;
        }

        bool alreadyExists = false;
        for (auto& p : projects) {
            if (p.path == projectFolder.string()) { alreadyExists = true; break; }
        }

        if (!alreadyExists) {
            ProjectEntry entry;
            entry.name = name.empty() ? projectFolder.filename().string() : name;
            entry.path = projectFolder.string();
            entry.favorite = false;
            entry.engineVersion = engineVersion.empty() ? std::to_string(APP_VERSION_MAJOR) + "." + std::to_string(APP_VERSION_MINOR) + "." + std::to_string(APP_VERSION_PATCH) : engineVersion;

            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
            localtime_s(&tm, &t);
            char dateBuf[32];
            strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M", &tm);
            entry.lastOpened = dateBuf;

            projects.push_back(entry);
            SaveProjects();
        }
    }
    }
    
    ImGui::SameLine();
    float sortLabelW = ImGui::CalcTextSize("Sort:").x + ImGui::GetStyle().ItemSpacing.x;
    float filterWidth = ImGui::GetContentRegionAvail().x - sortWidth - sortLabelW - ImGui::GetStyle().ItemSpacing.x * 3;
    ImGui::SetNextItemWidth(filterWidth);
    ImGui::InputTextWithHint("##filter", "Filter Projects", filterBuffer, sizeof(filterBuffer));
    ImGui::SameLine();
    ImGui::TextUnformatted("Sort:");
    ImGui::SameLine();
    const char* sortOptions[] = { "Favorite", "Last Edited", "Name", "Path" };
    ImGui::SetNextItemWidth(sortWidth);
    ImGui::Combo("##sort", &sortIndex, sortOptions, 4);
    ImGui::PopStyleVar(3);
    ImGui::Spacing();

    float mainHeight = ImGui::GetContentRegionAvail().y;
    float listWidth = ImGui::GetContentRegionAvail().x - rightPanelWidth - ImGui::GetStyle().ItemSpacing.x;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::BeginChild("ProjectList", ImVec2(listWidth, mainHeight), ImGuiChildFlags_Borders);
    DrawProjectList();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::SameLine();

    ImGui::BeginChild("ActionPanel", ImVec2(rightPanelWidth, 0), false);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    ImGui::Separator();
    bool hasSelection = (selectedIndex >= 0 && selectedIndex < (int)projects.size());
    if (!hasSelection) ImGui::BeginDisabled();
    
    if (ImGui::Button(ICON_FA_PENCIL " Edit", ImVec2(-1, 0))) RequestOpenProject(selectedIndex);
    
    if (ImGui::Button(ICON_FA_I_CURSOR " Rename", ImVec2(-1, 0))) {}
    
    if (ImGui::Button(ICON_FA_TRASH " Remove", ImVec2(-1, 0))) {
        removeConfirmIndex = selectedIndex;
    }

    if (!hasSelection) ImGui::EndDisabled();
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    ImGui::EndChild();

    if (openCreatePopup)
        ImGui::OpenPopup("Create Project");

    if (removeConfirmIndex >= 0)
        ImGui::OpenPopup("Remove Project");

    char versionText[128];
    snprintf(versionText, sizeof(versionText), "Ballistic Engine v%d.%d.%d.stable.official[%s]", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);
    float textW = ImGui::CalcTextSize(versionText).x;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textW - ImGui::GetStyle().WindowPadding.x);
    ImGui::TextDisabled("%s", versionText);
    
    DrawCreateProjectPopup();
    DrawRemoveProjectPopup();
    
    ImGui::End();
}

void EditorApplication::DrawProjectList()
{
    const float rowH = 64.0f;
    const float rowPad = 4.0f;
    const float rounding = 5.0f;
    const float pad = 8.0f;

    std::vector<int> sortedIndices;
    sortedIndices.reserve(projects.size());
    for (int i = 0; i < (int)projects.size(); i++) {
        if (filterBuffer[0] != '\0') {
            auto toLower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
            std::string f = toLower(filterBuffer);
            if (toLower(projects[i].name).find(f) == std::string::npos && toLower(projects[i].path).find(f) == std::string::npos)
                continue;
        }
        sortedIndices.push_back(i);
    }

    std::stable_sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
        if (sortIndex == 0) return projects[a].favorite > projects[b].favorite;
        if (sortIndex == 1) return projects[a].lastOpened > projects[b].lastOpened;
        if (sortIndex == 2) return projects[a].name < projects[b].name;
        if (sortIndex == 3) return projects[a].path < projects[b].path;
        return false;
    });

    if (sortedIndices.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        float textW = ImGui::CalcTextSize("No projects found.").x;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - textW) * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.5f);
        ImGui::TextUnformatted("No projects found.");
        ImGui::PopStyleColor();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    for (int idx : sortedIndices) {
        auto& p = projects[idx];
        int i = idx;

        if (filterBuffer[0] != '\0') {
            auto toLower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
            std::string f = toLower(filterBuffer);
            if (toLower(p.name).find(f) == std::string::npos && toLower(p.path).find(f) == std::string::npos)
                continue;
        }

        ImGui::Dummy(ImVec2(0, rowPad));

        bool selected = (selectedIndex == i);
        ImVec2 rowMin = ImGui::GetCursorScreenPos();
        float availW = ImGui::GetContentRegionAvail().x;
        ImVec2 rowMax = ImVec2(rowMin.x + availW, rowMin.y + rowH);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (selected)
            dl->AddRectFilled(rowMin, rowMax, IM_COL32(30, 80, 110, 255), rounding);
        else if (ImGui::IsMouseHoveringRect(rowMin, rowMax))
            dl->AddRectFilled(rowMin, rowMax, IM_COL32(255, 255, 255, 12), rounding);

        dl->AddLine(ImVec2(rowMin.x + pad, rowMax.y), ImVec2(rowMax.x - pad, rowMax.y), IM_COL32(255, 255, 255, 15));

        float thumbSize = rowH - pad * 2;
        float contentX = rowMin.x + pad * 3;
        float contentY = rowMin.y + pad;

        if (false) {
            dl->AddRectFilled(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(40, 40, 50, 255), 4.0f);
            dl->AddRect(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(80, 80, 90, 255), 4.0f);
        } else {
            dl->AddImage((ImTextureID)logoTextureID, ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize));
        }

        ImGui::PushID(i);

        float starSize = ImGui::GetTextLineHeight();
        float starX = rowMin.x + pad * 0.5f;
        float starY = rowMin.y + (rowH - starSize) * 0.5f;
        ImGui::SetCursorScreenPos(ImVec2(starX, starY));
        if (ImGui::InvisibleButton("##fav", ImVec2(starSize, starSize))) {
            projects[i].favorite = !projects[i].favorite;
            SaveProjects();
        }
        bool starHovered = ImGui::IsItemHovered();

        ImGui::SetCursorScreenPos(ImVec2(rowMin.x + starSize + pad, rowMin.y));
        if (ImGui::InvisibleButton("##row", ImVec2(availW - starSize - pad, rowH)))
            selectedIndex = i;
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
            RequestOpenProject(i);
        
        ImGui::PopID();

        float textX = contentX + thumbSize + pad;
        
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(starX, starY), p.favorite ? IM_COL32(255,180,0,255) : starHovered ? IM_COL32(255,255,255,255) : IM_COL32(80,80,80,255), ICON_FA_STAR);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, contentY), IM_COL32(220, 220, 220, 255), p.name.c_str());
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, ImVec2(textX, contentY + ImGui::GetTextLineHeight() + 3.0f), IM_COL32(110, 110, 110, 255), p.path.c_str());

        std::string meta = p.lastOpened;
        float metaW = ImGui::CalcTextSize(meta.c_str()).x;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(rowMax.x - metaW - pad, rowMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f), IM_COL32(100, 100, 100, 255), meta.c_str());
    }

    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
        selectedIndex = -1;

    ImGui::Dummy(ImVec2(0, rowPad));
    ImGui::PopStyleVar();
}

void EditorApplication::DrawCreateProjectPopup()
{
    bool open = true;
    if (ImGui::BeginPopupModal("Create Project", &open, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Project Name");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##createname", "My Project", createNameBuffer, sizeof(createNameBuffer));

        std::string folderName = createNameBuffer;
        std::replace(folderName.begin(), folderName.end(), ' ', '_');
        folderName.erase(std::remove_if(folderName.begin(), folderName.end(), [](char c) {
            return !std::isalnum(c) && c != '_' && c != '-';
        }), folderName.end());

        std::filesystem::path projectFolder = std::filesystem::path(createPathBuffer) / folderName;

        bool folderExistsOnDisk = !folderName.empty() && createPathBuffer[0] != '\0' && std::filesystem::exists(projectFolder);

        bool alreadyRegistered = false;
        for (auto& p : projects) {
            if (std::filesystem::path(p.path) == projectFolder) {
                alreadyRegistered = true;
                break;
            }
        }

        bool parentPathInvalid = createPathBuffer[0] != '\0' && !std::filesystem::exists(createPathBuffer);
        bool pathInvalid = parentPathInvalid || folderExistsOnDisk || alreadyRegistered;

        ImGui::Text("Project Path");
        if (pathInvalid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));
        ImGui::SetNextItemWidth(-80);
        ImGui::InputTextWithHint("##createpath", "C:/Projects", createPathBuffer, sizeof(createPathBuffer));
        if (pathInvalid) ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(-1, 0))) {
            std::string picked = FileDialog("Select Project Location", nullptr, true);
            if (!picked.empty())
                strncpy_s(createPathBuffer, sizeof(createPathBuffer), picked.c_str(), sizeof(createPathBuffer) - 1);
        }

        if (!folderName.empty() && createPathBuffer[0] != '\0') {
            ImGui::TextDisabled("Folder: %s", projectFolder.string().c_str());
        }

        if (pathInvalid) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (parentPathInvalid) ImGui::TextUnformatted("Path does not exist.");
            else if (alreadyRegistered) ImGui::TextUnformatted("Project already registered.");
            else ImGui::TextUnformatted("Project folder already exists.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Edit Now", &createEditNow);

        bool canCreate = createNameBuffer[0] != '\0' && createPathBuffer[0] != '\0' && !folderName.empty() && !pathInvalid;

        if (!canCreate) ImGui::BeginDisabled();
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            std::filesystem::create_directories(projectFolder);

            std::ofstream blstc(projectFolder / "project.blst");
            blstc << "name=" << createNameBuffer << "\n";
            blstc << "engine_version=" << APP_VERSION_MAJOR << "." << APP_VERSION_MINOR << "." << APP_VERSION_PATCH << "\n";

            ProjectEntry entry;
            entry.name = createNameBuffer;
            entry.path = projectFolder.string();
            entry.favorite = false;
            entry.engineVersion = std::to_string(APP_VERSION_MAJOR) + "." + std::to_string(APP_VERSION_MINOR) + "." + std::to_string(APP_VERSION_PATCH);

            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
            localtime_s(&tm, &t);
            char dateBuf[32];
            strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M", &tm);
            entry.lastOpened = dateBuf;

            projects.push_back(entry);
            SaveProjects();

            if (createEditNow)
                RequestOpenProject((int)projects.size() - 1);

            createNameBuffer[0] = '\0';
            createPathBuffer[0] = '\0';

            ImGui::CloseCurrentPopup();
        }
        if (!canCreate) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            createNameBuffer[0] = '\0';
            createPathBuffer[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (!open) {
        createNameBuffer[0] = '\0';
        createPathBuffer[0] = '\0';
    }
}

void EditorApplication::DrawRemoveProjectPopup()
{
    bool open = true;
    if (ImGui::BeginPopupModal("Remove Project", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (removeConfirmIndex >= 0 && removeConfirmIndex < (int)projects.size()) {
            ImGui::Text("Are you sure you want to remove:");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1,1,1,1), "%s", projects[removeConfirmIndex].name.c_str());
            ImGui::TextDisabled("%s", projects[removeConfirmIndex].path.c_str());
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "This will delete the project folder from disk.");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Yes, Delete", ImVec2(120, 0))) {
                std::filesystem::path projectPath2 = projects[removeConfirmIndex].path;
                if (std::filesystem::exists(projectPath2))
                    std::filesystem::remove_all(projectPath2);

                projects.erase(projects.begin() + removeConfirmIndex);
                if (selectedIndex == removeConfirmIndex)
                    selectedIndex = -1;
                else if (selectedIndex > removeConfirmIndex)
                    selectedIndex--;

                removeConfirmIndex = -1;
                SaveProjects();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                removeConfirmIndex = -1;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (!open)
        removeConfirmIndex = -1;
}

void EditorApplication::RequestOpenProject(int index)
{
    if (index < 0 || index >= (int)projects.size()) return;
    pendingOpenPath = projects[index].path;
    openProjectRequested = true;
    currentProjectIndex = index;
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

    ImGui::Begin("Test Panel");
    ImGui::Text("Ballistic Engine");
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Begin("Viewport");
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)finalTextureID, size);
    size = ImVec2((float)renderer.logoImage.extent.width, (float)renderer.logoImage.extent.height);
    ImGui::Image((ImTextureID)logoTextureID, size);
    size = ImVec2((float)renderer.logoLongImage.extent.width, (float)renderer.logoLongImage.extent.height);
    ImGui::Image((ImTextureID)logoLongTextureID, size);
    ImGui::End();
}
