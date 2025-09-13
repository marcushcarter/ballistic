#include "BEngine/engine.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE_Engine engine("Unordered Maps and Scenes");
    engine.bind();

    BE_Scene scene;

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    BE_Light light1;
    scene.lights.addLight(light1);

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.getFrameTime().frameCountFPS == 1) std::cout << engine.getFrameTime().fps << " FPS " << engine.getFrameTime().ms << " MS" << std::endl;

        scene.activeCamera->width = engine.width;
        scene.activeCamera->height = engine.height;
        scene.activeCamera->handleInputs(engine.getWindow(), engine.getFrameTime().dt);
        scene.activeCamera->updateViewMatrix();

        { // recompile shader
            auto shader = engine.resources().getShader("Default_Light");
            if (shader && glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) {
                engine.resources().getShader("Default_Light")->recompile("include/BEngine/shaders/core/sh_core_default.vert", "include/BEngine/shaders/core/sh_core_default.frag");
            }
        }

        if (glfwGetKey(engine.getWindow(), GLFW_KEY_1) == GLFW_PRESS) {
            scene.addCamera("Camera2");
        }

        // updates
        
        BE_Light lightnew2(1.0f, glm::vec3(0,0.5,0), glm::vec3(0), glm::vec3(1), std::sinf(glfwGetTime()) + 1.0f, 0.0f);
        scene.lights.updateLight(0, lightnew2);
        
        engine.beginRender();

        // rendering

        { // test scene model drawing
            auto shader = engine.resources().getShader("Default_Scene");
            auto mesh = engine.resources().getMesh("Test Scene");

            shader->activate();
            // scene.lights.updateActiveLightsForObject(glm::vec3(0,0,0), 5.0f);
            scene.lights.uploadToShader(shader->ID);
            scene.activeCamera->uploadToShader(shader->ID);
            glm::mat4 model = glm::mat4(1.0f);
            mesh->draw(*shader, model);
        }

        { // drawing lights
            auto shader = engine.resources().getShader("Default_Light");
            auto mesh = engine.resources().getMesh("Default_Cube");

            shader->activate();
            scene.activeCamera->uploadToShader(shader->ID);
            GLuint colorLoc = glGetUniformLocation(shader->ID, "uColor");
            for (int i = 0; i < scene.lights.lights.size(); i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(scene.lights.lights[i].position[0], scene.lights.lights[i].position[1], scene.lights.lights[i].position[2]));
                model = glm::scale(model, glm::vec3(0.1f));    
                glUniform4fv(colorLoc, 1, glm::value_ptr(scene.lights.lights[i].color));
                mesh->draw(*shader, model);
            }
        }
        
        { // drawing cameras
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            auto shader = engine.resources().getShader("Default_Light");
            auto mesh = engine.resources().getMesh("Default_Cube");

            shader->activate();
            scene.activeCamera->uploadToShader(shader->ID);
            glUniform4fv(glGetUniformLocation(shader->ID, "uColor"), 1, glm::value_ptr(glm::vec4(1)));
            for (auto& [key, camera] : scene.cameras) {
                // if (camera == scene.activeCamera) continue;
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