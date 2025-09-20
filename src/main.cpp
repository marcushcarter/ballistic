#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    engine.activeScene->lights().addLight("light1", 1);
    engine.activeScene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));
    engine.activeScene->lights().updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();

        engine.activeScene->activeCamera->updateViewMatrix();

        engine.viewport.get()->scene = engine.activeScene;
        engine.viewport.get()->camera = engine.activeScene->activeCamera;
        engine.viewport->scene->lights().updateGPU();
        engine.renderViewportTexture(*engine.viewport.get());

        // engine.activeScene->lights().updateGPU();

        engine.viewport->scene->lights().updateGPU();
        
        // engine.viewport.get()->framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        // engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"], false);

        editor.beginFrame();
        editor.showPanels();

        // ImGui::Begin("Inspector");
        // for (int i = 0; i < engine.activeScene->lights().lights.size(); i++) {
        //     auto& light = engine.activeScene->lights().lights[i];
        //     std::string name = "";
        //     for (const auto& [key, idx] : engine.activeScene->lights().lightLookup) {
        //         if (idx == i) {
        //             name = key;
        //             break;
        //         }
        //     }
        //
        //     std::string label = "Light " + std::to_string(i);
        //     // std::string label2 = engine.activeScene->lights().lightLookup[i];
        //
        //     if (ImGui::CollapsingHeader(name.c_str())) {
        //         ImGui::Text(name.c_str());
        //         if (ImGui::InputFloat("Type" + i, &light.position.w)) { engine.activeScene->lights().updateGPU(); }
        //         if (ImGui::DragFloat3("Position" + i, &light.position.x, 0.01f, -3.0f, 3.0f)) { engine.activeScene->lights().updateGPU(); }
        //         if (ImGui::DragFloat3("Direction" + i, &light.direction.x, 0.01f, -3.1416f, 3.1416f)) { engine.activeScene->lights().updateGPU(); }
        //         if (ImGui::ColorEdit3("Color" + i, &light.color.x)) { engine.activeScene->lights().updateGPU(); }
        //         if (ImGui::DragFloat("intensity" + i, &light.color.w, 0.01f, 0.0f, 5.0f)) { engine.activeScene->lights().updateGPU(); }
        //     }
        // }
        // ImGui::End();

        ImGui::Begin("Heirarchy");
        for (auto& [key, scene] : engine.scenes) {

            if (ImGui::TreeNode(key.c_str())) {
                if (ImGui::Button("Set Active")) { engine.activeScene = scene.get(); }

                for (int i = 0; i < scene->lights().lights.size(); i++) {
                    auto& light = scene->lights().lights[i];

                    std::string name = "";
                    for (const auto& [key, idx] : scene->lights().lightLookup) {
                        if (idx == i) {
                            name = key;
                            break;
                        }
                    }

                    if (ImGui::TreeNode(name.c_str())) {
                        if (ImGui::InputFloat("Type", &light.position.w)) { scene->lights().updateGPU(); }
                        if (ImGui::DragFloat3("Position", &light.position.x, 0.01f, -3.0f, 3.0f)) { scene->lights().updateGPU(); }
                        // if (ImGui::DragFloat3("Direction", &light.direction.x, 0.01f, -3.1416f, 3.1416f)) { scene->lights().updateGPU(); }
                        if (ImGui::ColorEdit3("Color", &light.color.x)) { scene->lights().updateGPU(); }
                        if (ImGui::DragFloat("intensity", &light.color.w, 0.01f, 0.0f)) { scene->lights().updateGPU(); }

                        ImGui::TreePop();
                    }

                }

                for (auto& [key, camera] : scene->cameras) {
                    if (ImGui::TreeNode(key.c_str())) {
                        if (ImGui::Button("Set Active")) { engine.activeScene->activeCamera = camera.get(); }
                        
                        if (ImGui::DragFloat3("Position", &camera->position.x, 0.01f, -3.0f, 3.0f)) {}
                        
                        ImGui::TreePop();
                    }
                }
                
                ImGui::TreePop();
            }
                
            // if (ImGui::CollapsingHeader("uqwbue")) {
            //     if (ImGui::Button(key.c_str() + 1)) { engine.activeScene = scene.get(); }

            //     for (int i = 0; i < scene->lights().lights.size(); i++) {
            //         auto& light = scene->lights().lights[i];

            //         std::string name = "";
            //         for (const auto& [key, idx] : scene->lights().lightLookup) {
            //             if (idx == i) {
            //                 name = key;
            //                 break;
            //             }
            //         }
                
            //         if (ImGui::CollapsingHeader(name.c_str())) {
            //             ImGui::Text(name.c_str());
            //             if (ImGui::InputFloat("Type" + i, &light.position.w)) { scene->lights().updateGPU(); }
            //             if (ImGui::DragFloat3("Position" + i, &light.position.x, 0.01f, -3.0f, 3.0f)) { scene->lights().updateGPU(); }
            //             if (ImGui::DragFloat3("Direction" + i, &light.direction.x, 0.01f, -3.1416f, 3.1416f)) { scene->lights().updateGPU(); }
            //             if (ImGui::ColorEdit3("Color" + i, &light.color.x)) { scene->lights().updateGPU(); }
            //             if (ImGui::DragFloat("intensity" + i, &light.color.w, 0.01f, 0.0f, 5.0f)) { scene->lights().updateGPU(); }
            //         }
            //     }

            // }

        }
        ImGui::End();

        ImGui::Begin("Hello, ImGui!");
        static ImVec2 lastSize = ImGui::GetWindowSize();
        ImVec2 size = ImGui::GetContentRegionAvail();
        bool resizing = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (!resizing) engine.viewport.get()->resize(size.x/2, size.y/2);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) engine.activeScene->activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        ImGui::Image((void*)(intptr_t) engine.viewport.get()->framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        editor.endFrame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}