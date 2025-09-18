#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    std::shared_ptr<BE::Scene> scene = std::make_shared<BE::Scene>();

    BE::Viewport vp1;
    vp1.scene = scene;
    vp1.camera = scene->cameras["Camera1"];
    vp1.resize(720, 450);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    scene->lights().addLight("light1", 1);
    scene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));
    scene->lights().updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();

        scene->activeCamera->updateViewMatrix();

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) { engine.resources().recompileShaders(); }

        engine.renderViewportTexture(vp1);
        
        // vp1.framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        // engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"], false);

        editor.beginFrame();
        editor.showPanels();

        ImGui::Begin("Inspector");
        if (ImGui::DragFloat3("Position", &scene->lights().getLight("light1")->position.x, 0.01f, -3.0f, 3.0f)) { scene->lights().updateGPU(); } // this runs whenever a value is changed
        if (ImGui::ColorEdit3("Color", &scene->lights().getLight("light1")->color.x)) { scene->lights().updateGPU(); }
        if (ImGui::DragFloat("intensity", &scene->lights().getLight("light1")->color.w, 0.01f, 0.0f, 2.0f)) { scene->lights().updateGPU(); }
        ImGui::End();

        ImGui::Begin("Hello, ImGui!");
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) scene->activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        ImGui::Image((void*)(intptr_t)vp1.framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        editor.endFrame();

        vp1.resize(size.x, size.y);

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}