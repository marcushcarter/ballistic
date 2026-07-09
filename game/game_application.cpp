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
    if (debug_menu_visible) {
        draw_menu_bar();
        dev_tools.draw_tools();
    }
#endif
}

void GameApplication::on_shutdown() {}

void GameApplication::draw_menu_bar()
{
    if (debug_menu_visible && ImGui::BeginMainMenuBar()) {

        dev_tools.draw_menu();

        // if (ImGui::BeginMenu("Profile Tools")) {

        //     // Mouse tools
        //     // -----------
        //     // AI >
        //     // Animations >
        //     // sequence >
        //     // scirpts >
        //     // rendergraph >
        //     // network >
        //     // quest >
        //     // world enumerators >
        //     // -----------
        //     // save settings
            
        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("Renderer")) {

        //     // wireframe
        //     // grid >
        //     // -----------
        //     // make swatch screenshot
        //     // 4x mode
        //     // -----------
        //     // reverse depth
        //     // -----------
        //     // renderbuffer xray
        //     // material reference colors
        //     // frame sync
        //     // -----------
        //     // scenegraph >
        //     // lens flares > 
        //     // renderpasses >
        //     // anise debugging >
        //     // post debugging >
        //     // ambient lighting >
        //     // reflections and cubemaps >
        //     // aa filtering >
        //     // debug display >
        //     // clous rendering >
        //     // lighting >
        //     // draw debugging >
        //     // streaming >
        //     // content instancing >
        //     // -----------
        //     // hotload shaders (CTRL+H)
            
        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("Performance")) {

        //     // fps
        //     // frame time graph
        //     // draw calls
        //     // triangles
        //     // memory usage
        //     // allocations per frame
        //     // entitiy count

        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("Effects")) {
        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("Sound")) {
        //     ImGui::EndMenu();
        // }

        // if (ImGui::BeginMenu("HUD")) {
        //     ImGui::EndMenu();
        // }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Open Documentation")) {
                ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

}