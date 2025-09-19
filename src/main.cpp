#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    std::unique_ptr<BE::Scene> scene = std::make_unique<BE::Scene>();


    BE::Viewport vp1;
    // vp1.scene = scene;
    // vp1.camera = scene->cameras["Camera1"].get();
    vp1.resize(720, 450);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    scene->lights().addLight("light1", 1);
    scene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));
    scene->lights().updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();

        scene->activeCamera->updateViewMatrix();

        engine.renderViewportTexture(vp1);
        
        // vp1.framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        // engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"], false);

        editor.beginFrame();
        editor.showPanels();

        ImGui::Begin("Inspector");
        if (ImGui::InputFloat("Type", &scene->lights().getLight("light1")->position.w)) { scene->lights().updateGPU(); }
        if (ImGui::DragFloat3("Position", &scene->lights().getLight("light1")->position.x, 0.01f, -3.0f, 3.0f)) { scene->lights().updateGPU(); } // this runs whenever a value is changed
        if (ImGui::DragFloat3("Direction", &scene->lights().getLight("light1")->direction.x, 0.01f, -3.1416f, 3.1416f)) { scene->lights().updateGPU(); }
        if (ImGui::ColorEdit3("Color", &scene->lights().getLight("light1")->color.x)) { scene->lights().updateGPU(); }
        if (ImGui::DragFloat("intensity", &scene->lights().getLight("light1")->color.w, 0.01f, 0.0f, 5.0f)) { scene->lights().updateGPU(); }
        if (ImGui::Button("Buttonw ac")) vp1.camera = scene->cameras["Camera1"].get();
        if (ImGui::Button("Buttonw asca")) vp1.scene = scene.get();
        ImGui::End();

        ImGui::Begin("Hello, ImGui!");
        static ImVec2 lastSize = ImGui::GetWindowSize();
        ImVec2 size = ImGui::GetContentRegionAvail();
        bool resizing = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (!resizing) vp1.resize(size.x, size.y);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) scene->activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        ImGui::Image((void*)(intptr_t)vp1.framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        editor.endFrame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}