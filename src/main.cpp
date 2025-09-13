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

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) {
            engine.resources().loadShaderDSL("include/BEngine/shaders/core/flat_color.dsl");
        }

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_1) == GLFW_PRESS) {
            scene.addCamera("Camera2");
        }

        // updates

        scene.lights().getLight("light1")->setIntensity(std::sinf(glfwGetTime()) + 1.0f);
        scene.lights().updateGPU();
        
        engine.beginRender();

        // rendering

        { // test scene model drawing
            auto shader = engine.resources().getShader("default_scene");
            auto mesh = engine.resources().getMesh("Test Scene");

            shader->activate();
            scene.lights().uploadToShader(shader->ID);
            scene.activeCamera->uploadToShader(shader->ID);
            glm::mat4 model = glm::mat4(1.0f);
            mesh->draw(*shader, model);
        }

        { // drawing lights
            auto shader = engine.resources().getShader("flat_color");
            auto mesh = engine.resources().getMesh("Default_Cube");

            shader->activate();
            scene.activeCamera->uploadToShader(shader->ID);
            GLuint colorLoc = glGetUniformLocation(shader->ID, "uColor");
            for (int i = 0; i < scene.lights().lights.size(); i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(scene.lights().lights[i].position[0], scene.lights().lights[i].position[1], scene.lights().lights[i].position[2]));
                model = glm::scale(model, glm::vec3(0.1f));    
                glUniform4fv(colorLoc, 1, glm::value_ptr(scene.lights().lights[i].color));
                mesh->draw(*shader, model);
            }
        }
        
        { // drawing cameras
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            auto shader = engine.resources().getShader("flat_color");
            auto mesh = engine.resources().getMesh("Default_Cube");

            shader->activate();
            scene.activeCamera->uploadToShader(shader->ID);
            glUniform4fv(glGetUniformLocation(shader->ID, "uColor"), 1, glm::value_ptr(glm::vec4(1)));
            for (auto& [key, camera] : scene.cameras) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(camera->position[0], camera->position[1], camera->position[2]));
                model = glm::scale(model, glm::vec3(0.25f));
                mesh->draw(*shader, model);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        engine.endFrame();
    }

    return 0;
}