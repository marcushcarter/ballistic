#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    BE::Viewport vp1;
    vp1.scene = engine.activeScene;
    vp1.camera = engine.activeScene->activeCamera;
    vp1.resize(720, 450);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    engine.activeScene->lights().addLight("light1", 1);
    engine.activeScene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));
    engine.activeScene->lights().updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();

        engine.activeScene->activeCamera->updateViewMatrix();

        engine.renderViewportTexture(vp1);
        
        // vp1.framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        // engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"], false);

        editor.beginFrame();
        editor.showPanels();

        ImGui::Begin("Inspector");
        if (ImGui::InputFloat("Type", &engine.activeScene->lights().getLight("light1")->position.w)) { engine.activeScene->lights().updateGPU(); }
        if (ImGui::DragFloat3("Position", &engine.activeScene->lights().getLight("light1")->position.x, 0.01f, -3.0f, 3.0f)) { engine.activeScene->lights().updateGPU(); } // this runs whenever a value is changed
        if (ImGui::DragFloat3("Direction", &engine.activeScene->lights().getLight("light1")->direction.x, 0.01f, -3.1416f, 3.1416f)) { engine.activeScene->lights().updateGPU(); }
        if (ImGui::ColorEdit3("Color", &engine.activeScene->lights().getLight("light1")->color.x)) { engine.activeScene->lights().updateGPU(); }
        if (ImGui::DragFloat("intensity", &engine.activeScene->lights().getLight("light1")->color.w, 0.01f, 0.0f, 5.0f)) { engine.activeScene->lights().updateGPU(); }
        // if (ImGui::Button("Set Cam")) vp1.camera = engine.activeScene->cameras["Camera1"].get();
        // if (ImGui::Button("Set Scene")) vp1.scene = engine.activeScene;
        ImGui::End();

        ImGui::Begin("Hello, ImGui!");
        static ImVec2 lastSize = ImGui::GetWindowSize();
        ImVec2 size = ImGui::GetContentRegionAvail();
        bool resizing = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (!resizing) vp1.resize(size.x/2, size.y/2);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) engine.activeScene->activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        ImGui::Image((void*)(intptr_t)vp1.framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        editor.endFrame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}