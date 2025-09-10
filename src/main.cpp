#include "include/engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE_Engine engine("Shadow Maps");
    engine.bind();

    BE_Mesh scene("Test Scene", {}, {}, {});
    scene.loadOBJ("res/models/scene.obj");

    BE_Mesh cube("Cube", {}, {}, {});
    cube.loadOBJ("res/models/cube.obj");

    BE_Texture texture1("fallback", "diffuse", 4, 4, BE::Default::FallbackTexture);

    BE_Shader shader("CubeShader", &BE::Default::SceneVertexSource, &BE::Default::SceneFragmentSource);
    BE_Shader colorshader("CubeShader", &BE::Default::SceneVertexSource, &BE::Default::ColorFragmentSource);
    BE_Camera camera("MainCam", engine.width, engine.height, 45.0f, 0.1f, 100.0f, {0,0.5,2}, {0,0,0});
    glfwSwapInterval(0);

    BE_LightManager lights(128);

    BE_Light light1;    
    lights.addLight(light1);
    
    // BE_Light light2;    
    // lights.addLight(light2);

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.frameTime.frameCountFPS == 1) std::cout << engine.frameTime.fps << " FPS " << engine.frameTime.ms << " MS" << std::endl;

        camera.handleInputs(engine.window, engine.frameTime.dt);
        camera.updateViewMatrix();

        // updates

        // glm::vec3 rainbowColor = glm::vec3(std::sinf(glfwGetTime() * 0.5f) * 0.5f + 0.5f, std::sinf(glfwGetTime() * 0.5f + 2.0943951f) * 0.5f + 0.5f, std::sinf(glfwGetTime() * 0.5f + 4.1887902f) * 0.5f + 0.5f);
        // BE_Light lightnew(1.0f, glm::vec3(std::sinf(glfwGetTime()), 0.5f, std::cosf(glfwGetTime())), glm::vec3(0), rainbowColor, 1.0f, 0.0f);
        // lights.updateLight(0, lightnew);
        
        BE_Light lightnew2(1.0f, glm::vec3(0,0.5,0), glm::vec3(0), glm::vec3(1), (std::sinf(glfwGetTime()) * 0.5f) + 0.5f, 0.0f);
        lights.updateLight(0, lightnew2);

        engine.beginRender();

        shader.activate();
        lights.updateActiveLightsForObject(glm::vec3(0,0,0), 5.0f);
        lights.uploadToShader(shader.ID);

        glm::mat4 model = glm::mat4(1.0f);
        camera.uploadToShader(shader.ID, model);
        scene.draw(shader);
        
        lights.draw(colorshader, cube, camera);

        engine.endFrame();
    }

    return 0;
}