#pragma once

namespace BE {
    namespace Default {

        inline const std::string DepthVertexSource = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;
        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
        )";

        inline const std::string SceneVertexSource = R"(
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

        inline const std::string SceneFragmentSource = R"(
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

        inline const std::string FallbackTexture =
            std::string(
                "\x00\x00\x00\xFF"
                "\xFF\xFF\xFF\xFF"
                "\xFF\xFF\xFF\xFF"
                "\x00\x00\x00\xFF", 
                16);

    }
};
