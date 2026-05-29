#include "editor_application.h"
#include "resources.h"

void EditorApplication::OnInit()
{
    window.SetEmbeddedIcon(IMG_ICON_COMP_PNG);
    window.SetTitlebarColor(0.15f, 0.15f, 0.15f);
    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
    };

    workspace.Load();
    
    imguiLayer.Create(renderer, window.glfwWindow);

    ImGuiIO& io = ImGui::GetIO();
    {
        HRSRC jbRes = FindResource(nullptr, MAKEINTRESOURCE(FONT_JETBRAINS_MONO_REGULAR_TTF), RT_RCDATA);
        HGLOBAL jbMem = LoadResource(nullptr, jbRes);
        DWORD jbSize = SizeofResource(nullptr, jbRes);
        void* jbData = IM_ALLOC(jbSize);
        memcpy(jbData, LockResource(jbMem), jbSize);
        ImFontConfig jbCfg;
        jbCfg.FontDataOwnedByAtlas = true;
        io.Fonts->AddFontFromMemoryTTF(jbData, (int)jbSize, 14.0f, &jbCfg);

        HRSRC faRes = FindResource(nullptr, MAKEINTRESOURCE(FONT_FA_SOLID_900_OTF), RT_RCDATA);
        HGLOBAL faMem = LoadResource(nullptr, faRes);
        DWORD faSize = SizeofResource(nullptr, faRes);
        void* faData = IM_ALLOC(faSize);
        memcpy(faData, LockResource(faMem), faSize);
        static const ImWchar faRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig faCfg;
        faCfg.MergeMode = true;
        faCfg.PixelSnapH = true;
        faCfg.FontDataOwnedByAtlas = true;
        io.Fonts->AddFontFromMemoryTTF(faData, (int)faSize, 14.0f, &faCfg, faRanges);
        io.Fonts->Build();
    }

    renderer.onSwapchainPass = [this](VkCommandBuffer cmd) {
        imguiLayer.RecordDraw(cmd);
    };

    finalTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        renderer.finalImage.GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    logoTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        splash.logoImage.GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    logoLongTextureID = ImGui_ImplVulkan_AddTexture(
        renderer.linearSampler.Get(),
        splash.logoLongImage.GetView(),
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

    projectManager.Start(workspace, logoLongTextureID, splash.logoLongImage.extent);

    onProjectLoadFailed = [this]() {
        inProjectManager = true;
    };
    
    LOG_DEBUG("Editor initialized");
}

void EditorApplication::OnUpdate()
{
    imguiLayer.NewFrame();

    if (inProjectManager) {
        window.SetTitle("Ballistic Engine - Project Manager");
        std::filesystem::path requested = projectManager.Draw();
        if (!requested.empty()) {
            inProjectManager = false;
            OpenProject(requested);
        }
    } else {
        window.SetTitle(("Ballistic Engine - " + project.name).c_str());

        EditorContext ctx{ project, renderer, workspace };
        ctx.finalTextureID = finalTextureID;

        editor.Update(ctx);
        editor.Draw(ctx);

        if (ctx.requestCloseProject)
            CloseProject();
    }

    imguiLayer.Render();
}

void EditorApplication::OnShutdown()
{
    renderer.device.Wait();
    workspace.Save();
    imguiLayer.Destroy();
    LOG_DEBUG("Editor shutdown");
}

void EditorApplication::OnProjectOpened(const std::filesystem::path& path)
{
    editor.OpenProject(path);
    workspace.TouchProject(path);
    LOG_INFO("Editing project: %s", path.string().c_str());
}

void EditorApplication::OnProjectClosed()
{
    editor.CloseProject(project.path);
    inProjectManager = true;
}
