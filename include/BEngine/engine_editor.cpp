#include "BEngine/engine_editor.hpp"

namespace BE {
    
Editor::Editor(Engine* enginePtr) : engine(enginePtr) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(engine->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Editor::~Editor() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::beginFrame() {
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // ImGui::DockSpaceOverViewport(
    //     ImGui::GetMainViewport(),
    //     ImGuiDockNodeFlags_PassthruCentralNode
    // );

    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.0f, 0.2f, 1.0f));

    window_flags |= 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Dockspace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
    ImGui::End();
    
    // ImGui::PopStyleColor();
}

void Editor::showPanels() {

    // === TOP PANEL === //

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { engine->closeWindow(); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Add Scene")) { std::string label = "Scene" + std::to_string(engine->scenes.size()+1); engine->addScene(label); }
            if (ImGui::MenuItem("Add Camera")) { std::string label = "Camera" + std::to_string(engine->activeScene->cameras.size()+1); engine->activeScene->addCamera(label); }
            if (ImGui::MenuItem("Add Light")) { std::string label = "Light" + std::to_string(engine->activeScene->lights().lights.size()+1);  engine->activeScene->lights().addLight(label, 1); }
            ImGui::EndMenu(); 
        };
        if (ImGui::BeginMenu("View")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Window")) { ImGui::EndMenu(); };
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Recompile Shaders")) { engine->resources().recompileShaders(); }
            if (ImGui::MenuItem("Update GPU")) { engine->viewport->scene->lights().updateGPU(); }
            ImGui::EndMenu(); 
        };
        ImGui::EndMainMenuBar();

    }

    // === PERFORMANCE == //

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Performance Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking );
    ImGui::Text("ENGINE FPS %.1f MS %.2f", engine->frameTime.fps, engine->frameTime.ms);
    ImGui::Text("IMGUI FPS %.1f MS %.2f", ImGui::GetIO().Framerate, 1000.0f/ImGui::GetIO().Framerate);
    ImGui::End();

    // === TERMINAL === //

    // === HEIRARCHY === //

    // === INSPECTOR === //

    // ImGui::ShowDemoWindow(nullptr);

    // ImGui::Begin("Hello, ImGui!");
    // ImGui::Text("This is a test window!");
    // ImVec2 size = ImGui::GetContentRegionAvail();
    // // ImGui::Image((void*)(intptr_t)vp1.framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
    // if (ImGui::Button("Click Me!")) {}
    // ImGui::End();

    // ImGui::Begin("Scene View");
    // ImGui::Text("Scene renders here");
    // ImGui::End();
    
    // ImGui::Begin("Console");
    // ImGui::Text("Logs Go Here");
    // ImGui::End();
}

void Editor::endFrame() {

    ImGui::Render();
    glViewport(0, 0, engine->width, engine->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

}; // BE namespace