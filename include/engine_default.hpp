#pragma once

namespace BE {
    namespace Default {

        inline const std::string DepthVertexShader = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;
        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
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