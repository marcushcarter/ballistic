#include "EditorApplication.h"
#include "Images.h"

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
    
    LOG_DEBUG("Editor initialized");
}

void EditorApplication::OnUpdate()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (inProjectManager)
        DrawProjectManager();
    else {
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

    ImGui::Render();
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

    std::ifstream file(cfgPath);
    std::string line;
    ProjectEntry current;
    bool hasCurrent = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            if (hasCurrent) projects.push_back(current);
            current = {};
            current.path = line.substr(1, line.size() - 2);
            hasCurrent = true;
        } else {
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            if (key == "name") current.name = val;
            else if (key == "favorite") current.favorite = (val == "true");
            else if (key == "last_opened") current.lastOpened = val;
            else if (key == "engine_version") current.engineVersion = val;
        }
    }
    if (hasCurrent) projects.push_back(current);
    
    for (auto& p : projects)
        LOG_INFO("Project: %s | %s | %s | %s | %s", p.name.c_str(), p.path.c_str(), p.lastOpened.c_str(), p.engineVersion.c_str(), p.favorite ? "fav" : "");
}

void EditorApplication::SaveProjects()
{
    std::ofstream file(roamingRoot / "projects.cfg");
    for (auto& p : projects) {
        file << "[" << p.path << "]\n";
        file << "name=" << p.name << "\n";
        file << "favorite=" << (p.favorite ? "true" : "false") << "\n";
        file << "last_opened=" << p.lastOpened << "\n";
        file << "engine_version=" << p.engineVersion << "\n\n";
    }
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
    ImGui::Button("Settings");
    ImGui::PopStyleColor(3);
    ImGui::Spacing();
    
    float mainBoxHeight = ImGui::GetContentRegionAvail().y - bottomBarHeight;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::BeginChild("MainBox", ImVec2(0, mainBoxHeight), ImGuiChildFlags_Borders);
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);
    ImGui::Button("+ Create");
    ImGui::SameLine();
    ImGui::Button("Import");
    ImGui::SameLine();
    float sortLabelW = ImGui::CalcTextSize("Sort:").x + ImGui::GetStyle().ItemSpacing.x;
    float filterWidth = ImGui::GetContentRegionAvail().x - sortWidth - sortLabelW - ImGui::GetStyle().ItemSpacing.x * 3;
    ImGui::SetNextItemWidth(filterWidth);
    ImGui::InputText("##filter", filterBuffer, sizeof(filterBuffer));
    ImGui::SameLine();
    ImGui::TextUnformatted("Sort:");
    ImGui::SameLine();
    static int sortIndex = 0;
    const char* sortOptions[] = { "Last Edited", "Name", "Path" };
    ImGui::SetNextItemWidth(sortWidth);
    ImGui::Combo("##sort", &sortIndex, sortOptions, 3);
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
    if (ImGui::Button("Edit", ImVec2(-1, 0))) inProjectManager = false;
    if (ImGui::Button("Rename", ImVec2(-1, 0))) {}
    ImGui::Spacing();
    if (ImGui::Button("Remove", ImVec2(-1, 0))) {}
    if (!hasSelection) ImGui::EndDisabled();
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    ImGui::EndChild();

    char versionText[128];
    snprintf(versionText, sizeof(versionText), "Ballistic Engine v%d.%d.%d.stable.official[%s]", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_COMMIT);
    float textW = ImGui::CalcTextSize(versionText).x;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textW - ImGui::GetStyle().WindowPadding.x);
    ImGui::TextDisabled("%s", versionText);
    
    ImGui::End();
}

// void EditorApplication::DrawProjectList()
// {

// }

void EditorApplication::DrawProjectList()
{
    const float rowH      = 64.0f;
    const float rowPad    = 4.0f;
    const float rounding  = 5.0f;
    const float pad       = 8.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    for (int i = 0; i < (int)projects.size(); i++) {
        auto& p = projects[i];

        if (filterBuffer[0] != '\0') {
            auto toLower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
            std::string f = toLower(filterBuffer);
            if (toLower(p.name).find(f) == std::string::npos &&
                toLower(p.path).find(f) == std::string::npos)
                continue;
        }

        ImGui::Dummy(ImVec2(0, rowPad));

        bool selected = (selectedIndex == i);
        ImVec2 rowMin = ImGui::GetCursorScreenPos();
        float  availW = ImGui::GetContentRegionAvail().x;
        ImVec2 rowMax = ImVec2(rowMin.x + availW, rowMin.y + rowH);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (selected)
            dl->AddRectFilled(rowMin, rowMax, IM_COL32(30, 80, 110, 255), rounding);
        else if (ImGui::IsMouseHoveringRect(rowMin, rowMax))
            dl->AddRectFilled(rowMin, rowMax, IM_COL32(255, 255, 255, 12), rounding);

        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##row", ImVec2(availW, rowH))) {
            selectedIndex = i;
            if (ImGui::IsMouseDoubleClicked(0))
                inProjectManager = false;
        }
        // ImGui::PopID();
        ImGui::PopID();

dl->AddLine(
    ImVec2(rowMin.x + pad, rowMax.y),
    ImVec2(rowMax.x - pad, rowMax.y),
    IM_COL32(255, 255, 255, 15));

        float thumbSize = rowH - pad * 2;
        float contentX  = rowMin.x + pad * 3;
        float contentY  = rowMin.y + pad;

        // thumbnail
        dl->AddRectFilled(
            ImVec2(contentX, contentY),
            ImVec2(contentX + thumbSize, contentY + thumbSize),
            IM_COL32(40, 40, 50, 255), 4.0f);
        dl->AddRect(
            ImVec2(contentX, contentY),
            ImVec2(contentX + thumbSize, contentY + thumbSize),
            IM_COL32(80, 80, 90, 255), 4.0f);

        // fav star
        dl->AddText(
            ImVec2(rowMin.x + pad * 0.5f, rowMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f),
            p.favorite ? IM_COL32(255, 180, 0, 255) : IM_COL32(80, 80, 80, 255),
            p.favorite ? "★" : "☆");

        // name
        float textX = contentX + thumbSize + pad;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
            ImVec2(textX, contentY),
            IM_COL32(220, 220, 220, 255),
            p.name.c_str());

        // path
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f,
            ImVec2(textX, contentY + ImGui::GetTextLineHeight() + 3.0f),
            IM_COL32(110, 110, 110, 255),
            p.path.c_str());

        // version + date right aligned
        std::string meta = p.engineVersion + "   " + p.lastOpened;
        float metaW = ImGui::CalcTextSize(meta.c_str()).x;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
            ImVec2(rowMax.x - metaW - pad, rowMin.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f),
            IM_COL32(100, 100, 100, 255),
            meta.c_str());
    }

    ImGui::Dummy(ImVec2(0, rowPad));
    ImGui::PopStyleVar();
}
