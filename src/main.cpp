#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    engine.resources().loadMesh("Test Scene", "res/models/scene.obj");

    editor.selectedAnchor = engine.activeScene->createAnchor();
    engine.activeScene->registry.tags[editor.selectedAnchor] = BE::TagComponent{"Test Scene", BE::AnchorType::None};
    engine.activeScene->registry.transforms[editor.selectedAnchor] = BE::TransformComponent{{0,0,0}, {0,0,0}, {1,1,1}};
    engine.activeScene->registry.meshes[editor.selectedAnchor] = BE::MeshComponent{engine.resources().meshes["Test Scene"], nullptr, nullptr};

    BE::Anchor light = engine.activeScene->createAnchor();
    engine.activeScene->registry.tags[light] = BE::TagComponent{"Light", BE::AnchorType::None};
    engine.activeScene->registry.transforms[light] = BE::TransformComponent{{0,0.5f,0}, {0,0,0}, {0.1,0.1,0.1}};
    engine.activeScene->registry.meshes[light] = BE::MeshComponent{engine.resources().meshes["default_cube"], nullptr, engine.resources().shaders["default_color"]};
    engine.activeScene->registry.lights[light] = BE::LightComponent{glm::vec3(1,1,1), 1.0f, 1};

    while(engine.isRunning()) {

        engine.beginFrame();

        engine.activeScene->activeCamera->updateViewMatrix();

        engine.viewport.get()->scene = engine.activeScene;
        engine.viewport.get()->camera = engine.activeScene->activeCamera;
        engine.renderViewportTexture(*engine.viewport.get());

        editor.Frame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}
