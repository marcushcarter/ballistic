#include "include/engine.hpp"
#include "engine_default.hpp"

#include <iostream>

struct BE_Light {
    glm::vec4 position; // xyz = position, w = type (1.0 = point light)
    glm::vec4 color;    // rgb = color, a = intensity
};

struct BE_LightBlock {
    alignas(16) int numLights;       // must align to 16 bytes in std140
    alignas(16) BE_Light lights[16]; // max 16 lights
};


const std::string vertexSrc = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTex;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    TexCoord = aTex;
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
}

)";

const std::string fragmentSrc = R"(
#version 460 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D diffuse0;

struct Light {
    vec4 position; // xyz = pos, w = type (1.0 = point)
    vec4 color;    // rgb = color, a = intensity
};

layout(std140, binding = 0) uniform LightBlock {
    int numLights;
    Light lights[16];
};

void main() {
    vec3 texColor = texture(diffuse0, TexCoord).rgb;
    vec3 norm = normalize(Normal);

    vec3 finalColor = vec3(0.2); // ambient fallback

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lights[i].position.xyz - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        float distance = length(lights[i].position.xyz - FragPos);
        float attenuation = 1.0 / (distance * distance + 0.01);

        finalColor += lights[i].color.rgb * lights[i].color.a * diff * attenuation;
    }

    FragColor = vec4(texColor * finalColor, 1.0);
}

)";

int main() {

    BE_Engine engine("Test Cube");
    engine.bind();

    // BE_Texture texture1("box diffuse", "res/textures/box.png", "diffuse", 0);
    // BE_Texture texture2("box specular", "res/textures/box_specular.png", "specular", 1);
    // std::vector<BE_Texture> textures = { texture1, texture2 };

    BE_Mesh cube("Cube", {}, {}, {});
    cube.loadOBJ("res/models/scene.obj");

    BE_Texture texture1("fallback", "diffuse", 2, 2, BE::Default::FallbackTexture);

    BE_Shader shader("CubeShader", &vertexSrc, &fragmentSrc);
    BE_Camera camera("MainCam", engine.width, engine.height, 45.0f, 0.1f, 100.0f, {0,0,3}, {0,0,-1});
    glfwSwapInterval(0);

    unsigned int lightUBO;
    glGenBuffers(1, &lightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(BE_LightBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO);

    // Fill the light block
    BE_LightBlock lightBlock{};
    lightBlock.numLights = 1;
    lightBlock.lights[0].position = glm::vec4(1.5f, 2.0f, 2.0f, 1.0f); // point light
    lightBlock.lights[0].color = glm::vec4(1.0f, 1.0f, 1.0f, 2.0f);      // white, intensity=2

    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(BE_LightBlock), &lightBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.frameTime.frameCountFPS == 1) std::cout << engine.frameTime.fps << " FPS " << engine.frameTime.ms << " MS" << std::endl;

        camera.handleInputs(engine.window, engine.frameTime.dt);
        camera.updateViewMatrix();

        // updates

        engine.beginRender();

        shader.activate();
        GLuint blockIndex = glGetUniformBlockIndex(shader.ID, "LightBlock");
        glUniformBlockBinding(shader.ID, blockIndex, 0);


        glm::mat4 model = glm::mat4(1.0f);
        camera.uploadToShader(shader.ID, model);
        cube.draw(shader);

        engine.endFrame();
    }

    engine.~BE_Engine();
    return 0;
}