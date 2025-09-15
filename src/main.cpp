#include "BEngine/engine.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("DSL file shader loading :-0");
    engine.bind();

    BE::Scene scene;

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    scene.lights().addLight("light1", 1);
    scene.lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.frameTime.frameCountFPS == 1) std::cout << engine.frameTime.fps << " FPS " << engine.frameTime.ms << " MS" << std::endl;

        scene.activeCamera->width = engine.width;
        scene.activeCamera->height = engine.height;
        scene.activeCamera->handleInputs(engine.getWindow(), engine.frameTime.dt);
        scene.activeCamera->updateViewMatrix();

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) { engine.resources().loadShaderDSL("include/BEngine/shaders/post/blit.dsl"); }
        if (glfwGetKey(engine.getWindow(), GLFW_KEY_1) == GLFW_PRESS) { /* scene.addCamera("Camera2"); */ scene.framebuffer.resize(engine.width, engine.height); }
        if (glfwGetKey(engine.getWindow(), GLFW_KEY_2) == GLFW_PRESS) { scene.framebuffer.resize(720, 450); }

        // updates

        scene.lights().getLight("light1")->setIntensity(std::sinf(glfwGetTime()) + 1.0f);
        scene.lights().getLight("light1")->setDirection(glm::vec3(0, std::sinf(glfwGetTime()), 0));
        scene.lights().updateGPU();

        // engine.beginRender();

        scene.render(engine.resources(), true);
        
        glViewport(0, 0, engine.width, engine.height);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.framebuffer.bindTexture(engine.resources().getShader("__blit")->ID, "screenTexture", 3);
        engine.resources().getMesh("__quad")->draw(*engine.resources().getShader("__blit"), glm::mat4(1));

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}