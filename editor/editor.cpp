#include <editor/editor.h>
#include <editor/viewport/viewport.h>
#include <editor/debugger/debugger.h>
#include <core/io/path.h>
#include <core/log/log.h>
#include <imgui.h>
#include <fstream>
#include <string>
// #include <iostream>

namespace ballistic {

Error Editor::create(const EditorContext& p_context)
{
    context = p_context;

    panels.push_back(std::make_unique<Viewport>());
    panels.push_back(std::make_unique<Debugger>());

    load_settings();
    return Error::Ok;
}

void Editor::destroy()
{
    save_settings();
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

void Editor::draw_panels() { for (auto& p : panels) p->draw(context); }

void Editor::draw_menu()
{
    if (panels.empty()) return;
    
    if (ImGui::BeginMenu("Editor")) {
        for (auto& p : panels) ImGui::MenuItem(p->name(), nullptr, &p->open);
        ImGui::Separator();
        if (ImGui::MenuItem("Close All")) for (auto& p : panels) p->open = false;
        ImGui::EndMenu();
    }
}

void Editor::load_settings()
{
    std::ifstream f(Paths::roaming_data() / "editor_settings.cfg");
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        size_t sp = line.rfind(' ');
        if (sp == std::string::npos) continue;
        std::string key = line.substr(0, sp);
        std::string value = line.substr(sp + 1);

        // if (key == "Editor.autosave_enabled")  { autosave_enabled = std::atoi(value.c_str()) != 0; continue; }
        // if (key == "Editor.autosave_interval") { autosave_interval = std::strtof(value.c_str(), nullptr); continue; }
        // if (key == "Editor.theme") { theme = std::atoi(value.c_str()); continue; }

        for (auto& p : panels) {
            std::string prefix = std::string(p->name()) + ".open";
            if (key == prefix) { p->open = std::atoi(value.c_str()) != 0; break; }
        }
    }
}

void Editor::save_settings()
{
    std::ofstream f(Paths::roaming_data() / "editor_settings.cfg");
    if (!f) return;

    // f << "Editor.autosave_enabled " << (autosave_enabled ? 1 : 0) << '\n';
    // f << "Editor.autosave_interval " << autosave_interval << '\n';
    // f << "Editor.theme " << theme << '\n';

    for (auto& p : panels)
        f << p->name() << ".open " << (p->open ? 1 : 0) << '\n';
}

}