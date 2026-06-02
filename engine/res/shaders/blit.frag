#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D uTextures[];
layout(set = 0, binding = 2) uniform sampler uSamplers[];

layout(push_constant) uniform PC {
    uint srcIndex;
    uint samplerIndex;
} pc;

layout(location = 0) in  vec2 vUV;
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = texture(sampler2D(uTextures[pc.srcIndex], uSamplers[pc.samplerIndex]), vUV);
}