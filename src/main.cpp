#include "BEngine/engine.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE_Engine engine("Shadow Maps");
    engine.bind();

    BE_Scene scene;

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");
    engine.resources().loadShader("Color", "include/BEngine/shaders/core/sh_core_default.vert", "include/BEngine/shaders/core/sh_color_uniform.frag");
    engine.resources().loadTexture("fallback", "diffuse", 4, 4, BE::Default::FallbackTexture);

    // BE_Camera camera("MainCam", 1440, 900, 45.0f, 0.1f, 100.0f, {0,0.5,2}, {0,0,0});

    BE_Light light1;
    scene.lights.addLight(light1);

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.getFrameTime().frameCountFPS == 1) std::cout << engine.getFrameTime().fps << " FPS " << engine.getFrameTime().ms << " MS" << std::endl;

        engine.activeCamera->handleInputs(engine.getWindow(), engine.getFrameTime().dt);
        engine.activeCamera->updateViewMatrix();

        {
            auto shader = engine.resources().getShaderPtr(0);
            if (shader && glfwGetKey(engine.getWindow(), GLFW_KEY_0) == GLFW_PRESS) {
                engine.resources().sceneShader->recompile("include/BEngine/shaders/core/sh_core_default.vert", "include/BEngine/shaders/core/sh_core_default.frag");
            }
        }

        // updates
        
        BE_Light lightnew2(1.0f, glm::vec3(0,0.5,0), glm::vec3(0), glm::vec3(1), std::sinf(glfwGetTime()) + 1.0f, 0.0f);
        scene.lights.updateLight(0, lightnew2);
        
        engine.beginRender();

        // rendering

        {
            auto& shader = engine.resources().sceneShader;
            auto mesh = engine.resources().getMeshPtr(0);

            shader->activate();
            scene.lights.updateActiveLightsForObject(glm::vec3(0,0,0), 5.0f);
            scene.lights.uploadToShader(shader->ID);
            engine.activeCamera->uploadToShader(shader->ID);
            glm::mat4 model = glm::mat4(1.0f);
            mesh->draw(*shader, model);
        }

        {
            auto shader = engine.resources().getShaderPtr(0);
            auto& mesh = engine.resources().cubeMesh;

            shader->activate();
            engine.activeCamera->uploadToShader(shader->ID);
            GLuint colorLoc = glGetUniformLocation(shader->ID, "uColor");
            for (int i = 0; i < scene.lights.lights.size(); i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(scene.lights.lights[i].position[0], scene.lights.lights[i].position[1], scene.lights.lights[i].position[2]));
                model = glm::scale(model, glm::vec3(0.1f));    
                glUniform4fv(colorLoc, 1, glm::value_ptr(scene.lights.lights[i].color));
                mesh->draw(*shader, model);
            }
        }

        engine.endFrame();
    }

    return 0;
}