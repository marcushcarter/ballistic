#include <editor/editor_mode/project_manager.h>
#include <editor/popup/popup_manager.h>
#include <core/project/project.h>
#include <core/io/path.h>
#include <core/base/error.h>
#include <imgui.h>
#include <fstream>

namespace ballistic {

Error ProjectManager::initialize()
{
    using enum Error;
    load_recents();
    return Ok;
}

void ProjectManager::shutdown()
{
    save_recents();
}

void ProjectManager::load_recents()
{
    recent.clear();
    std::ifstream f(Paths::local_data() / "recents");
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::filesystem::path path = line;
        if (!std::filesystem::exists(path / Project::FILE_NAME)) continue;
        recent.push_back({ path.filename().string(), path });
    }
}
 
void ProjectManager::save_recents()
{
    std::ofstream f(Paths::local_data() / "recents");
    if (!f) return;
    for (auto& e : recent) f << e.path.string() << '\n';
}
 
void ProjectManager::add_recent(const std::filesystem::path& p_root, std::string_view p_name)
{
    for (size_t i = 0; i < recent.size(); i++) {
        if (recent[i].path == p_root) {
            recent.erase(recent.begin() + i);
            break;
        }
    }
    recent.insert(recent.begin(), { std::string(p_name), p_root });
    save_recents();
}

void ProjectManager::on_update(EditorContext& ctx)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("ProjectManager", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);
 
    ImGui::TextUnformatted("Projects");
    ImGui::Separator();
 
    ImGui::BeginChild("list", ImVec2(ImGui::GetContentRegionAvail().x - 200.0f, 0));
 
    for (int i = 0; i < (int)recent.size(); ++i) {
        ImGui::PushID(i);
        if (ImGui::Selectable(recent[i].name.c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick)) {
            selected = i;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) ctx.open_project_callback(recent[i].path);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", recent[i].path.string().c_str());
        ImGui::PopID();
    }
 
    if (recent.empty()) ImGui::TextDisabled("No projects yet.");
    ImGui::EndChild();
 
    ImGui::SameLine();
    ImGui::BeginChild("actions");
    if (ImGui::Button("New Project", ImVec2(-1, 0))) ctx.popups->open("New Project");
    ImGui::BeginDisabled(selected < 0);
    if (ImGui::Button("Open", ImVec2(-1, 0))) ctx.open_project_callback(recent[selected].path);
    if (ImGui::Button("Remove", ImVec2(-1, 0))) {}
    if (ImGui::Button("Delete", ImVec2(-1, 0))) {}
    if (ImGui::Button("Export", ImVec2(-1, 0))) {}
    if (ImGui::Button("Run", ImVec2(-1, 0))) {}
    ImGui::EndDisabled();
    ImGui::EndChild();
 
    ImGui::End();
}

}