#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    BE::Anchor cube = engine.activeScene->createAnchor();
    engine.activeScene->registry.transforms[cube] = BE::TransformComponent{{0,0,0}, {0,0,0}, {1,1,1}};
    engine.activeScene->registry.meshes[cube] = BE::MeshComponent{engine.resources().meshes["__cube"], nullptr, engine.resources().shaders["__scene"]};

    engine.resources().materials["__default_material"]->uploadToShader(
        *engine.resources().shaders["__scene"].get()
    );

    // BE::Anchor light = engine.activeScene->createAnchor();
    // engine.activeScene->registry.transforms[light] = BE::TransformComponent{{-1,0,0}, {0,0,0}, {0.5,0.5,0.5}};

    // OLD

    engine.activeScene->lights().addLight("light1", 1);
    engine.activeScene->lights().getLight("light1")->setPosition(glm::vec3(0,0.5,0));
    engine.activeScene->lights().updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();

        engine.activeScene->activeCamera->updateViewMatrix();

        engine.viewport.get()->scene = engine.activeScene;
        engine.viewport.get()->camera = engine.activeScene->activeCamera;
        engine.viewport->scene->lights().updateGPU();
        engine.renderViewportTexture(*engine.viewport.get());

        // engine.activeScene->lights().updateGPU();

        engine.viewport->scene->lights().updateGPU();
        
        // engine.viewport.get()->framebuffer.bindTexture(engine.resources().shaders["__blit"]->ID, "screenTexture", 3);
        // engine.resources().meshes["__quad"]->draw(*engine.resources().shaders["__blit"], false);

        editor.Frame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}