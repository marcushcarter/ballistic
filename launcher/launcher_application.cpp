#include <launcher/launcher_application.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <IconsFontAwesome6.h>
#include <chrono>

namespace Ballistic::Launcher
{
void LauncherApplication::Run()
{
    projects.push_back({ "Ballistic Demo", "D:/Ballistic Games/Projects/BallisticDemo", "1.0.0", "2025-06-10 14:32", true });
    projects.push_back({ "Platformer Test", "D:/Ballistic Games/Projects/PlatformerTest", "1.0.0", "2025-06-09 09:11", false });
    projects.push_back({ "Dungeon Crawler", "D:/Ballistic Games/Projects/DungeonCrawler", "1.0.0", "2025-05-28 17:45", false });
    projects.push_back({ "Rain World Clone", "D:/Ballistic Games/Projects/RainWorldClone", "1.0.0", "2025-05-15 22:01", true });
    projects.push_back({ "Shader Playground", "D:/Ballistic Games/Projects/ShaderPlayground", "1.0.0", "2025-04-30 10:00", false });

    Setup();

    while (m_Running && !window.ShouldClose()) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        window.PollEvents();

        BuildLauncherUI();

        VkCommandBuffer cmd = BeginFrame();
        if (cmd != VK_NULL_HANDLE) {
            Render(cmd);
            EndFrame(cmd);
        }

        auto frameEnd = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(frameEnd - frameStart).count();
        double remaining = (1.0f / targetFPS) - elapsed;
        if (remaining > 0.0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(remaining));
        }
        lastFrameTime = frameEnd;
    }

    Shutdown();
}

void LauncherApplication::Setup()
{
    window.Create("Ballistic Launcher", 1280, 800);
    window.SetTitlebarColor(0.15f, 0.15f, 0.15f);
    window.Show();

    glfwSetFramebufferSizeCallback(window.GetNative(), [](GLFWwindow* win, int w, int h) {
        auto app = static_cast<LauncherApplication*>(glfwGetWindowUserPointer(win));
        app->viewportResized = true;
    });

    uint32_t glfwExtCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    std::vector<const char*> instanceRequiredExtensions;
    instanceRequiredExtensions.assign(glfwExt, glfwExt + glfwExtCount);
    #if !defined(NDEBUG)
        instanceRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    instance.Create("Ballistic Launcher", 1, 0, 0, instanceRequiredExtensions);
    surface.Create(instance.Get(), window.GetNative());
    
    std::vector<const char*> deviceExts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    physicalDevice.Pick(instance.Get(), surface.Get(), deviceExts);
    device.Create(physicalDevice.Get(), physicalDevice.graphicsFamily, physicalDevice.presentFamily, physicalDevice.transferFamily, deviceExts);
    graphicsQueue.Acquire(device.Get(), physicalDevice.graphicsFamily);
    presentQueue.Acquire(device.Get(), physicalDevice.presentFamily);

    swapchain.Create(physicalDevice.Get(), device.Get(), surface.Get(), { window.width, window.height }, VK_PRESENT_MODE_MAILBOX_KHR);
    std::vector<VkImage> rawImages = swapchain.GetImages();
    frameCount = static_cast<uint32_t>(rawImages.size());
    
    swapchainImages.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; i++)
        swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent);
    
    commandPool.Create(device.Get(), {
        .queueFamilyIndex = graphicsQueue.familyIndex,
        .resetable = true,
        .debugName = "GraphicsCommandPool"
    });
    
    commandBuffers.resize(frameCount);
    imageAvailable.resize(frameCount);
    renderFinished.resize(frameCount);
    inFlight.resize(frameCount);

    for (uint32_t i = 0; i < frameCount; i++) {
        commandBuffers[i].Allocate(device.Get(), commandPool.Get(), false, "CommandBuffer");
        imageAvailable[i].Create(device.Get(), "ImageAvailableSemaphore");
        renderFinished[i].Create(device.Get(), "RenderFinishedSemaphore");
        inFlight[i].Create(device.Get(), true, "InFlightFence");
    }
    
    descriptorPool.Create(device.Get(), {
        .samplers = 10,
        .combinedImageSamplers = 10,
        .sampledImages = 10,        
        .afterBind = false,
        .debugName = "ImGuiDescriptorPool"
    });

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
    pipelineRenderingInfo.pColorAttachmentFormats = &swapchain.format;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance.Get();
    initInfo.PhysicalDevice = physicalDevice.Get();
    initInfo.Device = device.Get();
    initInfo.QueueFamily = graphicsQueue.familyIndex;
    initInfo.Queue = graphicsQueue.Get();
    initInfo.DescriptorPool = descriptorPool.Get();
    initInfo.MinImageCount = frameCount;
    initInfo.ImageCount = frameCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplGlfw_InitForVulkan(window.GetNative(), true);
    ImGui_ImplVulkan_Init(&initInfo);
}

void LauncherApplication::Shutdown()
{
    device.Wait();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    descriptorPool.Destroy();

    for (uint32_t i = 0; i < frameCount; i++) {
        inFlight[i].Destroy();
        renderFinished[i].Destroy();
        imageAvailable[i].Destroy();
    }
    commandBuffers.clear();
    commandPool.Destroy();

    swapchainImages.clear();
    swapchain.Destroy();

    presentQueue = {};
    graphicsQueue = {};
    surface.Destroy();
    device.Destroy();
    instance.Destroy();

    window.Destroy();
}

void LauncherApplication::RecreateSwapchain()
{
    device.Wait();

    swapchain.Resize({ window.width, window.height }, VK_PRESENT_MODE_MAILBOX_KHR);
    auto rawImages = swapchain.GetImages();
    frameCount = (uint32_t)rawImages.size();

    swapchainImages.clear();
    swapchainImages.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; i++) {
        swapchainImages[i].WrapSwapchainImage(device.Get(), rawImages[i], swapchain.format, swapchain.extent);
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)window.width, (float)window.height);

    currentFrame = 0;
    imageIndex = 0;
}

VkCommandBuffer LauncherApplication::BeginFrame()
{
    if (viewportResized) {
        viewportResized = false;
        RecreateSwapchain();
    }

    inFlight[currentFrame].Wait();
    inFlight[currentFrame].Reset();

    VkResult result = vkAcquireNextImageKHR(device.Get(), swapchain.Get(), UINT64_MAX, imageAvailable[currentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) { return VK_NULL_HANDLE; }  

    commandBuffers[imageIndex].Reset();
    commandBuffers[imageIndex].Begin();
    return commandBuffers[imageIndex].Get();
}

void LauncherApplication::Render(VkCommandBuffer cmd)
{
    Vulkan::Image2D& sc = swapchainImages[imageIndex];
    sc.Transition(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    VkClearValue clearValue{};
    clearValue.color = {{1.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = sc.GetView();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = clearValue;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = swapchain.extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);

    sc.Transition(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);
}

void LauncherApplication::EndFrame(VkCommandBuffer cmd)
{
    commandBuffers[imageIndex].End();
    VkResult sub = graphicsQueue.Submit(cmd, imageAvailable[currentFrame].Get(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFinished[currentFrame].Get(), inFlight[currentFrame].Get());
    VkResult pres = presentQueue.Present(swapchain.Get(), imageIndex, renderFinished[currentFrame].Get());
    currentFrame = (currentFrame + 1) % frameCount;
}

void LauncherApplication::Close() { m_Running = false; }

void LauncherApplication::BuildLauncherUI()
{    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // ImGui::ShowDemoWindow();
    
    // ProjectRegistry& registry = workspace->registry;

    // openRequested = false;
    // pendingOpenPath.clear();

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

    // float logoH = 28.0f;
    // // float logoW = logoH * ((float)logoLongExtent.width / (float)logoLongExtent.height);
    // // ImGui::Image((ImTextureID)logoLongTextureID, ImVec2(logoW, logoH));
    // ImGui::SameLine();
    // ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90.0f);
    // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
    // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));
    // if (ImGui::Button(ICON_FA_WRENCH " Settings")) ImGui::OpenPopup("Settings");
    // ImGui::PopStyleColor(3);
    // ImGui::Spacing();

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
        // std::string picked = FileDialog("Import Project", nullptr, false, {
        //     { L"Ballistic Project", L"*.blst" },
        //     { L"All Files", L"*.*" }
        // });

        // if (!picked.empty()) {
        //     std::filesystem::path blstPath(picked);
        //     std::filesystem::path projectFolder = blstPath.parent_path();

        //     std::string name, engineVersion;
        //     try {
        //         auto blst = toml::parse_file(blstPath.string());
        //         name = blst["name"].value_or<std::string>("");
        //         engineVersion = blst["engine_version"].value_or<std::string>("");
        //     } catch (...) {}

        //     if (registry.Find(projectFolder) == nullptr) {
        //         ProjectEntry entry;
        //         entry.name = name.empty() ? projectFolder.filename().string() : name;
        //         entry.path = projectFolder.string();
        //         entry.favorite = false;
        //         entry.engineVersion = engineVersion.empty() ? std::to_string(APP_VERSION_MAJOR) + "." + std::to_string(APP_VERSION_MINOR) + "." + std::to_string(APP_VERSION_PATCH) : engineVersion;
        //         entry.lastOpened = FormatTimestampNow();

        //         registry.Add(entry);
        //         registry.Save();
        //     }
        // }
    }

    ImGui::SameLine();
    float sortLabelW = ImGui::CalcTextSize("Sort:").x + ImGui::GetStyle().ItemSpacing.x;
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
        // RequestOpen(selectedIndex);
    }

    if (ImGui::Button(ICON_FA_I_CURSOR " Rename", ImVec2(-1, 0))) {
        renameIndex = selectedIndex;
        strncpy_s(renameBuffer, sizeof(renameBuffer), projects[selectedIndex].name.c_str(), sizeof(renameBuffer) - 1);
    }
    
    if (ImGui::Button(ICON_FA_TRASH " Remove", ImVec2(-1, 0))) {
        // removeConfirmIndex = selectedIndex;
    }

    bool canRun = hasSelection && std::filesystem::exists(std::filesystem::path(projects[selectedIndex].path) / "ballistic_game.exe");
    if (!canRun) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_PLAY " Run", ImVec2(-1, 0))) {
        // std::filesystem::path projectRoot = registry.entries[selectedIndex].path;   // string → path here
        // std::wstring args = L"\"" + projectRoot.wstring() + L"\"";
        // LaunchProcess(projectRoot / "ballistic_game.exe", projectRoot, args);
    }
    if (!canRun) ImGui::EndDisabled();

    if (!hasSelection) ImGui::EndDisabled();
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    ImGui::EndChild();

    if (openCreatePopup) ImGui::OpenPopup("Create Project");
    if (removeConfirmIndex >= 0) ImGui::OpenPopup("Remove Project");
    if (renameIndex >= 0) ImGui::OpenPopup("Rename Project");

    char versionText[128];
    snprintf(versionText, sizeof(versionText), "Ballistic Engine v%d.%d.%d.stable.official[%s]", 1, 0, 0, "APP_COMMIT");
    ImVec2 textSize = ImGui::CalcTextSize(versionText);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth()  - textSize.x - ImGui::GetStyle().WindowPadding.x);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - textSize.y - ImGui::GetStyle().WindowPadding.y);
    ImGui::TextDisabled("%s", versionText);

    // DrawCreatePopup();
    // DrawRemovePopup();
    // DrawRenamePopup();
    // DrawSettingsPopup();

    ImGui::End();

    ImGui::Render();
}

void LauncherApplication::DrawList()
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
            if (toLower(projects[i].name).find(filterStr) == std::string::npos &&
                toLower(projects[i].path).find(filterStr) == std::string::npos)
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

        dl->AddRectFilled(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(40, 40, 50, 255), 4.0f);
        dl->AddRect(ImVec2(contentX, contentY), ImVec2(contentX + thumbSize, contentY + thumbSize), IM_COL32(80, 80, 90, 255), 4.0f);

        ImGui::PushID(idx);

        float starSize = ImGui::GetTextLineHeight();
        float starX = rowMin.x + pad * 0.5f;
        float starY = rowMin.y + (rowH - starSize) * 0.5f;
        ImGui::SetCursorScreenPos(ImVec2(starX, starY));
        if (ImGui::InvisibleButton("##fav", ImVec2(starSize, starSize)))
            p.favorite = !p.favorite;
        bool starHovered = ImGui::IsItemHovered();

        ImGui::SetCursorScreenPos(ImVec2(rowMin.x + starSize + pad, rowMin.y));
        if (ImGui::InvisibleButton("##row", ImVec2(availW - starSize - pad, rowH)))
            selectedIndex = idx;
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {}
            // RequestOpen(idx);

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
}