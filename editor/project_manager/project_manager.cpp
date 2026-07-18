#include <editor/project_manager/project_manager.h>
#include <core/project/project.h>
#include <core/io/path.h>
#include <core/log/log.h>
#include <imgui.h>
#include <fstream>

namespace ballistic {

void ProjectManager::load_recent()
{
    recent.clear();
    std::ifstream f(Paths::local_data() / "projects.cfg");
    if (!f) return;
 
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t sp = line.find(' ');
        if (sp == std::string::npos) continue;
 
        std::string key = line.substr(0, sp);
        bool fav = (key == "project.fav");
        if (key != "project" && !fav) continue;
 
        std::filesystem::path p = line.substr(sp + 1);
        if (!std::filesystem::exists(p / Project::FILE_NAME)) continue;
        recent.push_back({ p.filename().string(), p, fav });
    }
 
    sort_entries();
}
 
void ProjectManager::save_recent()
{
    std::ofstream f(Paths::local_data() / "projects.cfg");
    if (!f) return;
    for (const Entry& e : recent)
        f << (e.favorite ? "project.fav " : "project ") << e.path.string() << '\n';
}
 
void ProjectManager::add_recent(const std::filesystem::path& p_root, std::string_view p_name)
{
    bool was_favorite = false;
    for (size_t i = 0; i < recent.size(); ++i)
        if (recent[i].path == p_root) { was_favorite = recent[i].favorite; recent.erase(recent.begin() + i); break; }
 
    recent.insert(recent.begin(), { std::string(p_name), p_root, was_favorite });
    sort_entries();
    save_recent();
}
 
void ProjectManager::sort_entries()
{
    std::stable_sort(recent.begin(), recent.end(), [](const Entry& a, const Entry& b) { return a.favorite > b.favorite; });
}

void ProjectManager::on_update()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("ProjectManager", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);
 
    ImGui::TextUnformatted("Projects");
    ImGui::Separator();
 
    ImGui::BeginChild("list", ImVec2(ImGui::GetContentRegionAvail().x - 200.0f, 0));
 
    int toggle = -1;
 
    for (int i = 0; i < (int)recent.size(); ++i) {
        ImGui::PushID(i);
 
        ImGui::PushStyleColor(ImGuiCol_Text, recent[i].favorite ? ImVec4(1.0f, 0.7f, 0.0f, 1.0f) : ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::SmallButton("*")) toggle = i;
        ImGui::PopStyleColor();
        ImGui::SameLine();
 
        if (ImGui::Selectable(recent[i].name.c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick)) {
            selected = i;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                open_requested = true;
                open_path = recent[i].path;
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", recent[i].path.string().c_str());
        ImGui::PopID();
    }
 
    if (toggle >= 0) {
        recent[toggle].favorite = !recent[toggle].favorite;
 
        std::filesystem::path sel_path;
        if (selected >= 0 && selected < (int)recent.size()) sel_path = recent[selected].path;
 
        sort_entries();
 
        selected = -1;
        if (!sel_path.empty())
            for (int i = 0; i < (int)recent.size(); ++i)
                if (recent[i].path == sel_path) { selected = i; break; }
 
        save_recent();
    }
 
    if (recent.empty()) ImGui::TextDisabled("No projects yet.");
    ImGui::EndChild();
 
    ImGui::SameLine();
    ImGui::BeginChild("actions");
    if (ImGui::Button("New Project", ImVec2(-1, 0))) {
 
        std::filesystem::path root = Paths::local_data() / "TestProject";
        root = "D:/TestBallistic";
 
        if (Project::create(root, "TestProject") == Error::Ok) {
            add_recent(root, "TestProject");
            open_requested = true;
            open_path = root;
        }
    }
    ImGui::BeginDisabled(selected < 0);
    if (ImGui::Button("Open", ImVec2(-1, 0))) { open_requested = true; open_path = recent[selected].path; }
    if (ImGui::Button("Remove", ImVec2(-1, 0))) {
        recent.erase(recent.begin() + selected);
        selected = -1;
        save_recent();
    }
    ImGui::EndDisabled();
    ImGui::EndChild();
 
    ImGui::End();
}

}