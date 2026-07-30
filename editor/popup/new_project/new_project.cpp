#include <editor/popup/new_project/new_project.h>
#include <core/project/project.h>
#include <core/io/path.h>
#include <imgui.h>

namespace ballistic {

void NewProjectPopup::draw_contents(EditorContext& ctx)
{
    if (ImGui::Button("Create Project")) {
        std::filesystem::path root = Paths::local_data() / "TestProject";
        root = "D:/TestBallistic";
        Project::create(root, "TestProject");
        ctx.open_project_callback(root);
        open = false;
        ImGui::CloseCurrentPopup();
    }
}

}