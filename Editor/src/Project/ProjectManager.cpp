#include "ProjectManager.h"
#include "../FileDialog.h"

void ProjectManager::Start(const std::filesystem::path& root, VkDescriptorSet tex, VkExtent2D extent)
{
    roamingRoot = root;
    logoLongTextureID = tex;
    logoLongExtent = extent;
}

std::filesystem::path ProjectManager::Draw()
{
    openRequested = false;
    pendingOpenPath.clear();

    const float rightPanelWidth = 150.0f;
    const float bottomBarHeight = 20.0f;
    const float sortWidth = 200.0f;

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
    float logoW = logoH * ((float)logoLongExtent.width / (float)logoLongExtent.height);
    ImGui::Image((ImTextureID)logoLongTextureID, ImVec2(logoW, logoH));
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));
    if (ImGui::Button(ICON_FA_WRENCH " Settings")) ImGui::OpenPopup("Settings");
    ImGui::PopStyleColor(3);
    ImGui::Spacing();

    bool openCreatePopup = false;
    float mainBoxHeight  = ImGui::GetContentRegionAvail().y - bottomBarHeight;
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

            std::string name, engineVersion;
            try {
                auto blst = toml::parse_file(blstPath.string());
                name = blst["name"].value_or<std::string>("");
                engineVersion = blst["engine_version"].value_or<std::string>("");
            } catch (...) {}

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
                Save();
            }
        }
    }

    ImGui::SameLine();
    float sortLabelW  = ImGui::CalcTextSize("Sort:").x + ImGui::GetStyle().ItemSpacing.x;
    float filterWidth = ImGui::GetContentRegionAvail().x - sortWidth - sortLabelW - ImGui::GetStyle().ItemSpacing.x * 3;
    ImGui::SetNextItemWidth(filterWidth);
    ImGui::InputTextWithHint("##filter", "Filter Projects", filterBuffer, sizeof(filterBuffer));
    ImGui::SameLine();
    ImGui::TextUnformatted("Sort:");
    ImGui::SameLine();
    const char* sortOptions[] = { "Last Edited", "Name", "Path" };
    ImGui::SetNextItemWidth(sortWidth);
    ImGui::Combo("##sort", &sortIndex, sortOptions, 3);
    ImGui::PopStyleVar(3);
    ImGui::Spacing();

    float mainHeight = ImGui::GetContentRegionAvail().y;
    float listWidth = ImGui::GetContentRegionAvail().x - rightPanelWidth - ImGui::GetStyle().ItemSpacing.x;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::BeginChild("ProjectList", ImVec2(listWidth, mainHeight), ImGuiChildFlags_Borders);
    DrawList();
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::SameLine();

    ImGui::BeginChild("ActionPanel", ImVec2(rightPanelWidth, 0), false);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    ImGui::Separator();
    bool hasSelection = (selectedIndex >= 0 && selectedIndex < (int)projects.size());
    if (!hasSelection) ImGui::BeginDisabled();

    if (ImGui::Button(ICON_FA_PENCIL " Edit", ImVec2(-1, 0))) {
        RequestOpen(selectedIndex);
    }

    if (ImGui::Button(ICON_FA_I_CURSOR " Rename", ImVec2(-1, 0))) {
        renameIndex = selectedIndex;
        strncpy_s(renameBuffer, sizeof(renameBuffer), projects[selectedIndex].name.c_str(), sizeof(renameBuffer) - 1);
    }
    
    if (ImGui::Button(ICON_FA_TRASH " Remove", ImVec2(-1, 0))) {
        removeConfirmIndex = selectedIndex;
    }

    if (!hasSelection) ImGui::EndDisabled();
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    ImGui::EndChild();

    if (openCreatePopup) ImGui::OpenPopup("Create Project");
    if (removeConfirmIndex >= 0) ImGui::OpenPopup("Remove Project");
    if (renameIndex >= 0) ImGui::OpenPopup("Rename Project");

    char versionText[128];
    snprintf(versionText, sizeof(versionText), "Ballistic Engine v%d.%d.%d.stable.official[%s]", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);
    ImVec2 textSize = ImGui::CalcTextSize(versionText);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth()  - textSize.x - ImGui::GetStyle().WindowPadding.x);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - textSize.y - ImGui::GetStyle().WindowPadding.y);
    ImGui::TextDisabled("%s", versionText);

    DrawCreatePopup();
    DrawRemovePopup();
    DrawRenamePopup();
    DrawSettingsPopup();

    ImGui::End();

    return pendingOpenPath;
}

void ProjectManager::Load()
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

void ProjectManager::Save()
{
    toml::table root;
    toml::array arr;

    for (auto& p : projects) {
        toml::table entry;
        entry.insert("path", p.path);
        entry.insert("favorite", p.favorite);
        entry.insert("last_opened", p.lastOpened);
        arr.push_back(entry);
    }

    root.insert("projects", arr);
    std::ofstream file(roamingRoot / "projects.cfg");
    file << root;
}

void ProjectManager::DrawList()
{
    const float rowH = 64.0f;
    const float rowPad = 4.0f;
    const float rounding = 5.0f;
    const float pad = 8.0f;

    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    std::string filterStr = toLower(filterBuffer);

    std::vector<int> sortedIndices;
    sortedIndices.reserve(projects.size());
    for (int i = 0; i < (int)projects.size(); i++) {
        if (!filterStr.empty()) {
            if (toLower(projects[i].name).find(filterStr) == std::string::npos && toLower(projects[i].path).find(filterStr) == std::string::npos)
                continue;
        }
        sortedIndices.push_back(i);
    }

    std::stable_sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
        if (projects[a].favorite != projects[b].favorite) return projects[a].favorite > projects[b].favorite;
        if (sortIndex == 0) return projects[a].lastOpened > projects[b].lastOpened;
        if (sortIndex == 1) return projects[a].name < projects[b].name;
        if (sortIndex == 2) return projects[a].path < projects[b].path;
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

        ImGui::Dummy(ImVec2(0, rowPad));

        bool selected = (selectedIndex == idx);
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

        if (true) {
            dl->AddRectFilled(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(40, 40, 50, 255), 4.0f);
            dl->AddRect(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(80, 80, 90, 255), 4.0f);
        } else {
            dl->AddImage((ImTextureID)logoLongTextureID, ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize));
        }

        ImGui::PushID(idx);

        float starSize = ImGui::GetTextLineHeight();
        float starX = rowMin.x + pad * 0.5f;
        float starY = rowMin.y + (rowH - starSize) * 0.5f;
        ImGui::SetCursorScreenPos(ImVec2(starX, starY));
        if (ImGui::InvisibleButton("##fav", ImVec2(starSize, starSize))) {
            p.favorite = !p.favorite;
            Save();
        }
        bool starHovered = ImGui::IsItemHovered();

        ImGui::SetCursorScreenPos(ImVec2(rowMin.x + starSize + pad, rowMin.y));
        if (ImGui::InvisibleButton("##row", ImVec2(availW - starSize - pad, rowH)))
            selectedIndex = idx;
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
            RequestOpen(idx);

        ImGui::PopID();

        float textX = contentX + thumbSize + pad;

        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(starX, starY), p.favorite ? IM_COL32(255,180,0,255) : starHovered ? IM_COL32(255,255,255,255) : IM_COL32(80,80,80,255), ICON_FA_STAR);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, contentY), IM_COL32(220, 220, 220, 255), p.name.c_str());
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, ImVec2(textX, contentY + ImGui::GetTextLineHeight() + 3.0f), IM_COL32(110, 110, 110, 255), p.path.c_str());

        float metaW = ImGui::CalcTextSize(p.lastOpened.c_str()).x;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(rowMax.x - metaW - pad, rowMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f), IM_COL32(100, 100, 100, 255), p.lastOpened.c_str());
    }

    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
        selectedIndex = -1;

    ImGui::Dummy(ImVec2(0, rowPad));
    ImGui::PopStyleVar();
}

void ProjectManager::DrawCreatePopup()
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
        bool alreadyRegistered  = false;
        for (auto& p : projects) {
            if (std::filesystem::path(p.path) == projectFolder) { alreadyRegistered = true; break; }
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

        if (!folderName.empty() && createPathBuffer[0] != '\0')
            ImGui::TextDisabled("Folder: %s", projectFolder.string().c_str());

        if (pathInvalid) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (parentPathInvalid) ImGui::TextUnformatted("Path does not exist.");
            else if (alreadyRegistered) ImGui::TextUnformatted("Project already registered.");
            else ImGui::TextUnformatted("Project folder already exists.");
            ImGui::PopStyleColor();
        }

        ImGui::TextUnformatted("Version Control:");
        ImGui::SameLine();
        const char* sortOptions[] = { "None", "Git" };
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##sort", &createVersionControlIndex, sortOptions, 2);
        ImGui::Spacing();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Checkbox("Edit Now", &createEditNow);

        bool canCreate = createNameBuffer[0] != '\0' && createPathBuffer[0] != '\0' && !folderName.empty() && !pathInvalid;
        if (!canCreate) ImGui::BeginDisabled();
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            std::filesystem::create_directories(projectFolder);
            std::filesystem::create_directories(projectFolder / "Assets" / "Textures");
            std::filesystem::create_directories(projectFolder / "Assets" / "Models");
            std::filesystem::create_directories(projectFolder / "Assets" / "Audio");
            std::filesystem::create_directories(projectFolder / "Assets" / "Scripts");
            std::filesystem::create_directories(projectFolder / "Scenes");
            std::filesystem::create_directories(projectFolder / ".ballistic");
            SetFileAttributesA((projectFolder / ".ballistic").string().c_str(), FILE_ATTRIBUTE_HIDDEN);

            toml::table blst;
            blst.insert("name", std::string(createNameBuffer));
            blst.insert("engine_version", std::format("{}.{}.{}", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH));
            std::ofstream blstFile(projectFolder / "project.blst");
            blstFile << blst;

            if (createVersionControlIndex == 1) {
                std::ofstream gitignore(projectFolder / ".gitignore");
                gitignore << "# Ballistic Engine\n";
                gitignore << ".ballistic/\n";

                std::ofstream gitattributes(projectFolder / ".gitattributes");
                gitattributes << "* text=auto eol=lf\n\n";
                gitattributes << "# Binary assets\n";
                gitattributes << "*.png binary\n";
                gitattributes << "*.jpg binary\n";
                gitattributes << "*.jpeg binary\n";
                gitattributes << "*.tga binary\n";
                gitattributes << "*.wav binary\n";
                gitattributes << "*.mp3 binary\n";
                gitattributes << "*.ogg binary\n";
                gitattributes << "*.fbx binary\n";
                gitattributes << "*.blend binary\n\n";
                gitattributes << "# Engine files\n";
                gitattributes << "*.blst text eol=lf\n";
                gitattributes << "*.glsl text eol=lf\n";
                gitattributes << "*.hlsl text eol=lf\n";
            }

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
            Save();

            if (createEditNow) RequestOpen((int)projects.size() - 1);
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

void ProjectManager::DrawRemovePopup()
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
                std::filesystem::path projectPath = projects[removeConfirmIndex].path;
                if (std::filesystem::exists(projectPath)) std::filesystem::remove_all(projectPath);

                projects.erase(projects.begin() + removeConfirmIndex);
                if (selectedIndex == removeConfirmIndex) selectedIndex = -1;
                else if (selectedIndex >  removeConfirmIndex) selectedIndex--;

                removeConfirmIndex = -1;
                Save();
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

    if (!open) removeConfirmIndex = -1;    
}

void ProjectManager::DrawRenamePopup()
{
    bool open = true;
    if (ImGui::BeginPopupModal("Rename Project", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (renameIndex >= 0 && renameIndex < (int)projects.size()) {
            ImGui::Text("New Name");
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##rename", "Project Name", renameBuffer, sizeof(renameBuffer));
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool canRename = renameBuffer[0] != '\0' && projects[renameIndex].name != renameBuffer;
            if (!canRename) ImGui::BeginDisabled();
            if (ImGui::Button("Rename", ImVec2(120, 0))) {
                projects[renameIndex].name = renameBuffer;

                // update project.blst
                std::filesystem::path blstPath = std::filesystem::path(projects[renameIndex].path) / "project.blst";
                if (std::filesystem::exists(blstPath)) {
                    try {
                        auto blst = toml::parse_file(blstPath.string());
                        blst.insert_or_assign("name", std::string(renameBuffer));
                        std::ofstream f(blstPath);
                        f << blst;
                    } catch (...) {}
                }

                Save();
                renameIndex = -1;
                ImGui::CloseCurrentPopup();
            }
            if (!canRename) ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                renameIndex = -1;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
    if (!open) renameIndex = -1;
}

void ProjectManager::DrawSettingsPopup()
{
    bool open = true;
    if (ImGui::BeginPopupModal("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Settings");
        ImGui::Separator();

        // TODO
        
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void ProjectManager::RequestOpen(int index)
{
    if (index < 0 || index >= (int)projects.size()) return;
    pendingOpenPath = projects[index].path;
    openRequested   = true;
}
