#include "BEngine/engine.hpp"

// #include "include/BEngine/imgui/imgui.h"
// #include "include/BEngine/imgui/imgui_impl_glfw.h"
// #include "include/BEngine/imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    engine.bind();
    
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // ImGui::StyleColorsDark();

    // ImGui_ImplGlfw_InitForOpenGL(engine.getWindow(), true);
    // ImGui_ImplOpenGL3_Init("#version 460");

    std::shared_ptr<BE::Scene> scene = std::make_shared<BE::Scene>();

    BE::Viewport vp1;
    vp1.scene = scene;
    vp1.camera = scene->cameras["Camera1"];
    vp1.resize(720, 450);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    scene->lights().addLight("light1", 1);
    scene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));

    scene->addCamera("camcam");
    scene->activeCamera = scene->cameras["Camera1"];

    while(engine.isRunning()) {

        engine.beginFrame();

        vp1.resize(engine.width/2, engine.height/2);
        
        if (engine.frameTime.frameCountFPS == 1) std::cout << engine.frameTime.fps << " FPS " << engine.frameTime.ms << " MS" << std::endl;

        scene->activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        scene->activeCamera->updateViewMatrix();

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) { engine.resources().recompileShaders(); }
        if (glfwGetKey(engine.getWindow(), GLFW_KEY_1) == GLFW_PRESS) { scene->activeCamera = scene->cameras["camcam"]; }
        if (glfwGetKey(engine.getWindow(), GLFW_KEY_2) == GLFW_PRESS) { scene->activeCamera = scene->cameras["Camera1"]; }

        // updates

        scene->lights().getLight("light1")->setIntensity(std::sinf(glfwGetTime()) + 1.0f);
        scene->lights().getLight("light1")->setDirection(glm::vec3(0, std::sinf(glfwGetTime()), 0));
        scene->lights().updateGPU();

        engine.renderViewport(vp1);
        
        glViewport(0, 0, engine.width, engine.height);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vp1.framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"]);

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // ImGui::Begin("Hello, ImGui!");
        // ImGui::Text("This is a test window!");
        // ImVec2 size = ImGui::GetContentRegionAvail();
        // ImGui::Image((void*)(intptr_t)vp1.framebuffer.texture, size);
        // if (ImGui::Button("Click Me!")) {}

        // ImGui::End();

        // ImGui::Render();
        // glViewport(0, 0, engine.width, engine.height);
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(engine.getWindow());
    }

    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    return 0;
}