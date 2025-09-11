#include "include/engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE_Engine engine("Shadow Maps");
    engine.bind();

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");
    engine.resources().loadMesh("Cube", "res/models/cube.obj");
    
    engine.resources().loadShader("Scene", &BE::Default::SceneVertexSource, &BE::Default::SceneFragmentSource);
    engine.resources().loadShader("Color", &BE::Default::SceneVertexSource, &BE::Default::ColorFragmentSource);

    engine.resources().loadTexture("fallback", "diffuse", 4, 4, BE::Default::FallbackTexture);

    BE_Camera camera("MainCam", 1440, 900, 45.0f, 0.1f, 100.0f, {0,0.5,2}, {0,0,0});

    BE_LightManager lights(128);
    BE_Light light1;  
    lights.addLight(light1);

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.getFrameTime().frameCountFPS == 1) std::cout << engine.getFrameTime().fps << " FPS " << engine.getFrameTime().ms << " MS" << std::endl;

        camera.handleInputs(engine.getWindow(), engine.getFrameTime().dt);
        camera.updateViewMatrix();

        // updates

        // glm::vec3 rainbowColor = glm::vec3(std::sinf(glfwGetTime() * 0.5f) * 0.5f + 0.5f, std::sinf(glfwGetTime() * 0.5f + 2.0943951f) * 0.5f + 0.5f, std::sinf(glfwGetTime() * 0.5f + 4.1887902f) * 0.5f + 0.5f);
        // BE_Light lightnew(1.0f, glm::vec3(std::sinf(glfwGetTime()), 0.5f, std::cosf(glfwGetTime())), glm::vec3(0), rainbowColor, 1.0f, 0.0f);
        // lights.updateLight(0, lightnew);
        
        BE_Light lightnew2(1.0f, glm::vec3(0,0.5,0), glm::vec3(0), glm::vec3(1), std::sinf(glfwGetTime()) + 1.0f, 0.0f);
        // BE_Light lightnew2(0.0f, glm::vec3(0,0.5,0), glm::vec3(0, -0.5, 0), glm::vec3(1), 5.0f, 0.0f);
        lights.updateLight(0, lightnew2);
        
        glm::mat4 model = glm::mat4(1.0f);

        engine.beginRender();

        // rendering
        
        engine.resources().getShaderPtr(0)->activate();
        lights.updateActiveLightsForObject(glm::vec3(0,0,0), 5.0f);
        lights.uploadToShader(engine.resources().getShaderPtr(0)->ID);
        camera.uploadToShader(engine.resources().getShaderPtr(0)->ID);
        engine.resources().getMeshPtr(0)->draw(*engine.resources().getShaderPtr(0), model);

        lights.draw(*engine.resources().getShaderPtr(1), *engine.resources().getMeshPtr(1), camera);

        engine.endFrame();
    }

    return 0;
}