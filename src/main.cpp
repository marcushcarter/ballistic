#include "BEngine/engine.hpp"
#include "BEngine/engine_editor.hpp"

#include <iostream>
#include <cmath>

int main() {

    BE::Engine engine("Engine");
    BE::Editor editor(&engine);

    while(engine.isRunning()) {

        engine.beginFrame();

        engine.editorCamera.orbit.x += engine.frameTime.dt;
        engine.editorCamera.orbit.y = sinf(glfwGetTime())/2;
        
        engine.viewport->scene = engine.activeScene;

        engine.render();

        editor.Frame();

        glfwSwapBuffers(engine.getWindow());
    }

    return 0;
}
