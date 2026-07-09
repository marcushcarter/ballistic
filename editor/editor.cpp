#include <editor/editor.h>
#include <core/dev_tools/dev_systems.h>
#include <core/io/path.h>
#include <core/log/log.h>
#include <imgui.h>
#include <fstream>
#include <string>
#include <iostream>

namespace ballistic {

Error Editor::create(const EditorContext& p_context)
{
    context = p_context;
    load_layout();
    return Error::Ok;
}

void Editor::destroy()
{
    save_layout();
}

void Editor::update(float p_dt)
{
    (void)p_dt;
}

void Editor::draw()
{
    begin_dockspace();
    draw_menu_bar();
    viewport.draw(context);
    context.dev->debug_console.draw();
}

void Editor::begin_dockspace()
{
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imguiViewport->WorkPos);
    ImGui::SetNextWindowSize(imguiViewport->WorkSize);
    ImGui::SetNextWindowViewport(imguiViewport->ID);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
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

    ImGui::PopStyleVar(3);

    ImGui::DockSpace(ImGui::GetID("MainDockSpace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void Editor::draw_menu_bar()
{
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Viewport", nullptr, &viewport.open);
            ImGui::Separator();
            ImGui::MenuItem("Debug Console", nullptr, &context.dev->debug_console.open);
            ImGui::Separator();
            if (ImGui::MenuItem("Close All")) {
                viewport.open = false;
                context.dev->debug_console.open = false;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void Editor::load_layout() {
    std::filesystem::path path = Paths::roaming_data() / "editor_panels.cfg";
    if (path.empty()) return;
    std::ifstream f(path);
    if (!f) return;
    std::string key;
    int val;
    while (f >> key >> val) {
        if (key == "viewport") viewport.open = (val != 0);
        else if (key == "debug_console") context.dev->debug_console.open = (val != 0);
    }
}

void Editor::save_layout() {
    std::filesystem::path path = Paths::roaming_data() / "editor_panels.cfg";
    if (path.empty()) return;
    std::ofstream f(path);
    if (!f) return;
    f << "viewport " << (viewport.open ? 1 : 0) << "\n";
    f << "debug_console " << (context.dev->debug_console.open ? 1 : 0) << "\n";
}

}