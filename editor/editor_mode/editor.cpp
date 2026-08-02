#include <editor/editor_mode/editor.h>
#include <core/rendering/renderer.h>
#include <editor/panel/viewport/viewport.h>
#include <editor/panel/console/console.h>
#include <editor/panel/profiler/profiler.h>
#include <editor/panel/memory_profiler/memory_profiler.h>
#include <core/rendering/render_path/editor_render_path.h>
#include <editor/editor_settings.h>
#include <imgui.h>

namespace ballistic {

Error Editor::initialize()
{
    using enum Error;

    panels.push_back(std::make_unique<Viewport>());
    panels.push_back(std::make_unique<Console>());
    panels.push_back(std::make_unique<Profiler>());
    panels.push_back(std::make_unique<MemoryProfiler>());

    return Ok;
}

void Editor::shutdown()
{
    panels.clear();
}

void Editor::_begin_dockspace()
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

void Editor::on_update(EditorContext& ctx, float)
{
    _begin_dockspace();
    for (auto& p : panels) p->draw(ctx);
}

void Editor::draw_menu()
{
    if (panels.empty()) return;
    if (ImGui::BeginMenu("Panels")) {
        for (auto& p : panels) ImGui::MenuItem(p->name(), nullptr, &p->open);
        ImGui::Separator();
        if (ImGui::MenuItem("Close All")) for (auto& p : panels) p->open = false;
        ImGui::EndMenu();
    }
}

void Editor::take_screenshot(EditorContext& ctx)
{
    ctx.render_path->screenshot.requested = true;
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