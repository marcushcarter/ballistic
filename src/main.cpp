#include "include/engine.hpp"

#include <iostream>

const std::string vertexSrc = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTex;

uniform mat4 uMVP;

out vec3 fColor;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    fColor = aColor;
}
)";

const std::string fragmentSrc = R"(
#version 460 core
in vec3 fColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(fColor, 1.0);
}
)";

int main() {

    BE_Engine engine("Test Cube");
    engine.bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    std::vector<BE_Vertex> vertices = {
        {{-0.5f, -0.5f, -0.5f}, {0,0,0}, {1,0,0}, {0,0}},
        {{ 0.5f, -0.5f, -0.5f}, {0,0,0}, {0,1,0}, {1,0}},
        {{ 0.5f,  0.5f, -0.5f}, {0,0,0}, {0,0,1}, {1,1}},
        {{-0.5f,  0.5f, -0.5f}, {0,0,0}, {1,1,0}, {0,1}},
        {{-0.5f, -0.5f,  0.5f}, {0,0,0}, {1,0,1}, {0,0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,0,0}, {0,1,1}, {1,0}},
        {{ 0.5f,  0.5f,  0.5f}, {0,0,0}, {1,1,1}, {1,1}},
        {{-0.5f,  0.5f,  0.5f}, {0,0,0}, {0,0,0}, {0,1}}
    };

    std::vector<GLuint> indices = {
        0,1,2, 2,3,0,
        1,5,6, 6,2,1,
        5,4,7, 7,6,5,
        4,0,3, 3,7,4,
        3,2,6, 6,7,3,
        4,5,1, 1,0,4
    };

    BE_Mesh cube("Cube", vertices, indices, {});
    BE_Shader shader("CubeShader", &vertexSrc, &fragmentSrc);
    BE_Camera camera("MainCam", engine.width, engine.height, 45.0f, 0.1f, 100.0f, {0,0,3}, {0,0,-1});

    while(engine.isRunning()) {
        // engine.beginFrame();

        // engine.beginRender();
        // engine.endFrame();

        engine.beginFrame();

        camera.handleInputs(engine.window, engine.frameTime.dt);
        camera.updateViewMatrix();

        engine.beginRender();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // fix


        glm::mat4 model = glm::mat4(1.0f);
        camera.uploadToShader(shader.ID, model);

        cube.draw(shader);

        engine.endFrame();
    }

    engine.~BE_Engine();
    return 0;
}