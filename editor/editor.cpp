#include <editor/editor.h>
#include <core/rendering/renderer.h>

#include <editor/viewport/viewport.h>
#include <editor/console/console.h>
#include <editor/settings/settings.h>
#include <editor/profiler/profiler.h>
#include <editor/memory_profiler/memory_profiler.h>

#include <core/rendering/render_path/editor_render_path.h>
#include <editor/editor_settings.h>
#include <imgui.h>

namespace ballistic {

Error Editor::create(const EditorContext& p_context)
{
    using enum Error;

    context = p_context;
    if (!context.settings) return Failed;

    panels.push_back(std::make_unique<Viewport>());
    panels.push_back(std::make_unique<Console>());
    panels.push_back(std::make_unique<Settings>());
    panels.push_back(std::make_unique<Profiler>());
    panels.push_back(std::make_unique<MemoryProfiler>());

    apply_settings();
    return Ok;
}

void Editor::destroy()
{
    store_settings();
    panels.clear();
    close_project_requested = false;
}

void Editor::on_update(float p_dt)
{
    (void)p_dt;
    begin_dockspace();
    draw_panels();
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
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground
    );

    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID("MainDockSpace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void Editor::draw_panels() { for (auto& p : panels) p->draw(context); }

void Editor::draw_menu()
{
    if (panels.empty()) return;
    
    if (ImGui::BeginMenu("Project")) {
        ImGui::Separator();
        if (ImGui::MenuItem("Close Project")) close_project_requested = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Editor")) {
        for (auto& p : panels) ImGui::MenuItem(p->name(), nullptr, &p->open);
        ImGui::Separator();
        if (ImGui::MenuItem("Close All")) for (auto& p : panels) p->open = false;
        ImGui::EndMenu();
    }
}

void Editor::apply_settings()
{
    for (auto& p : panels) {
        auto it = panel_open.find(p->name());
        if (it != panel_open.end()) p->open = it->second;
    }
}

void Editor::store_settings()
{
    for (auto& p : panels) panel_open[p->name()] = p->open;
}

}