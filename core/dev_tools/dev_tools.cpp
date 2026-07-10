#include <core/dev_tools/dev_tools.h>
#include <core/dev_tools/xray/xray.h>
#include <core/dev_tools/profiler/claude_profiler.h>
#include <core/dev_tools/profiler/profiler.h>
#include <imgui.h>

namespace ballistic {

Error DevTools::create(const DevContext& p_context)
{
    using enum Error;
    context = p_context;
    
    panels.push_back(std::make_unique<Xray>());
    panels.push_back(std::make_unique<ClaudeProfiler>());
    panels.push_back(std::make_unique<Profiler>());

    return Ok;
}

void DevTools::destroy()
{

}

void DevTools::draw_panels(bool p_editor) {
    for (auto& p : panels) {
        if (p_editor && !p->show_in_editor()) continue;
        p->draw(context);
    }
}

void DevTools::draw_menu(bool p_editor)
{
    if (panels.empty()) return;
    
    if (ImGui::BeginMenu("Tools")) {
        for (auto& p : panels) {
            if (p_editor && !p->show_in_editor()) continue;
            ImGui::MenuItem(p->name(), nullptr, &p->open);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close All")) for (auto& p : panels) p->open = false;
        ImGui::EndMenu();
    }
}

}