#include <game/game_application.h>
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

namespace ballistic {

Error GameApplication::on_init() { return Error::Ok; }

void GameApplication::on_update(float p_dt)
{
    (void)p_dt;

    renderer.request_size(window.width, window.height);

#if BALLISTIC_DEV_TOOLS
    dev_tools.draw_panels();
    
    if (ImGui::BeginMainMenuBar()) {
        dev_tools.draw_menu();

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Open Documentation")) {
                ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
#endif
}

void GameApplication::on_shutdown() {}

}