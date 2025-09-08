#include "include/engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <cmath>

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
uniform int numLights;

struct Light {
    vec4 position; // xyz = pos/dir, w = type
    vec4 color;    // rgb = color, a = intensity
    vec4 extra;    // spot direction/cutoff if needed
};

layout(std430, binding = 0) buffer LightBlock {
    Light lights[];
};

void main() {
    vec3 texColor = texture(diffuse0, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    vec3 finalColor = vec3(0.2);

    for (int i = 0; i < numLights; i++) {
        vec3 lightColor = lights[i].color.rgb * lights[i].color.a;
        float diff = 0.0;

        if (lights[i].position.w == 0.0) {
            // Directional
            vec3 lightDir = normalize(-lights[i].position.xyz);
            diff = max(dot(norm, lightDir), 0.0);
        } 
        else if (lights[i].position.w == 1.0) {
            // Point
            vec3 lightDir = normalize(lights[i].position.xyz - FragPos);
            float distance = length(lights[i].position.xyz - FragPos);
            float attenuation = 1.0 / (distance * distance);
            diff = max(dot(norm, lightDir), 0.0) * attenuation;
        } 
        else if (lights[i].position.w == 2.0) {
            // Spot
            vec3 lightDir = normalize(lights[i].position.xyz - FragPos);
            float theta = dot(lightDir, normalize(-lights[i].extra.xyz));
            if (theta > lights[i].extra.w) {
                float distance = length(lights[i].position.xyz - FragPos);
                float attenuation = 1.0 / (distance * distance);
                diff = max(dot(norm, lightDir), 0.0) * attenuation;
            }
        }

        finalColor += lightColor * diff;
    }

    FragColor = vec4(texColor * finalColor, 1.0);
}
)";

int main() {

    BE_Engine engine("Test Cube");
    engine.bind();

    BE_Mesh cube("Cube", {}, {}, {});
    cube.loadOBJ("res/models/scene.obj");

    BE_Texture texture1("fallback", "diffuse", 2, 2, BE::Default::FallbackTexture);

    BE_Shader shader("CubeShader", &vertexSrc, &fragmentSrc);
    BE_Camera camera("MainCam", engine.width, engine.height, 45.0f, 0.1f, 100.0f, {0,0,3}, {0,0,-1});
    glfwSwapInterval(0);

    BE_LightManager lights;

    BE_Light dirLight{};
    dirLight.position = glm::vec4(0, -1, 0, 0);
    dirLight.color = glm::vec4(1, 1, 1, 0);
    lights.lights.push_back(dirLight);

    BE_Light pointLight{};
    pointLight.position = glm::vec4(0, 0.5, 2, 1);
    pointLight.color = glm::vec4(1, 0.8, 0.6, 1);
    lights.lights.push_back(pointLight);

    lights.updateGPU();

    while(engine.isRunning()) {

        engine.beginFrame();
        
        if (engine.frameTime.frameCountFPS == 1) std::cout << engine.frameTime.fps << " FPS " << engine.frameTime.ms << " MS" << std::endl;

        camera.handleInputs(engine.window, engine.frameTime.dt);
        camera.updateViewMatrix();

        // updates

        lights.lights[1].position = glm::vec4(std::sinf(glfwGetTime()), 0.5f, std::cosf(glfwGetTime()), 1);
        lights.lights[1].color = glm::vec4(
            std::sinf(glfwGetTime() * 0.5f) * 0.5f + 0.5f, 
            std::sinf(glfwGetTime() * 0.5f + 2.0943951f) * 0.5f + 0.5f, 
            std::sinf(glfwGetTime()*0.5f + 4.1887902f) * 0.5f + 0.5f, 
            1
        );

        lights.updateGPU();

        engine.beginRender();

        shader.activate();
        lights.uploadToShader(shader.ID);
        glm::mat4 model = glm::mat4(1.0f);
        camera.uploadToShader(shader.ID, model);
        cube.draw(shader);

        engine.endFrame();
    }

    engine.~BE_Engine();
    return 0;
}