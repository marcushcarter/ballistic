#include <game/game_application.h>
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

namespace ballistic {

Error GameApplication::on_init() { return Error::Ok; }

void GameApplication::on_update(float p_dt)
{
    (void)p_dt;
    renderer.set_size(window.width, window.height);

#if BALLISTIC_DEV_TOOLS
    if (debug_menu_visible) {
        draw_menu_bar();

        dev_systems.renderbuffer_xray.draw(renderer.graph, dev_systems.texture_cache);
        dev_systems.debug_console.draw();
    }
#endif
}

void GameApplication::on_shutdown() {}

void GameApplication::draw_menu_bar()
{
    if (debug_menu_visible && ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("RenderBuffer XRay", nullptr, &dev_systems.renderbuffer_xray.open);
            ImGui::MenuItem("Debug Console", nullptr, &dev_systems.debug_console.open);
            ImGui::Separator();
            if (ImGui::MenuItem("Close All")) {
                dev_systems.renderbuffer_xray.open = false;
                dev_systems.debug_console.open = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {

            // Mouse tools
            // -----------
            // AI >
            // Animations >
            // sequence >
            // scirpts >
            // rendergraph >
            // network >
            // quest >
            // world enumerators >
            // -----------
            // save settings
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Renderer")) {

            // wireframe
            // grid >
            // -----------
            // make swatch screenshot
            // 4x mode
            // -----------
            // reverse depth
            // -----------
            if (ImGui::MenuItem("Add RenderBuffer XRay")) dev_systems.renderbuffer_xray.open = true;
            // material reference colors
            // frame sync
            // -----------
            // scenegraph >
            // lens flares > 
            // renderpasses >
            // anise debugging >
            // post debugging >
            // ambient lighting >
            // reflections and cubemaps >
            // aa filtering >
            // debug display >
            // clous rendering >
            // lighting >
            // draw debugging >
            // streaming >
            // content instancing >
            // -----------
            // hotload shaders (CTRL+H)
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Performance")) {

            // fps
            // frame time graph
            // draw calls
            // triangles
            // memory usage
            // allocations per frame
            // entitiy count

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Effects")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Sound")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("HUD")) {
            ImGui::EndMenu();
        }

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