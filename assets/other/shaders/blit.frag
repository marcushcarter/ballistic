#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform texture2D uTextures[];
layout(set = 0, binding = 2) uniform sampler uSamplers[];

vec4 SampleBindless(uint texIndex, uint sampIndex, vec2 uv) {
    return texture(sampler2D(uTextures[nonuniformEXT(texIndex)], uSamplers[nonuniformEXT(sampIndex)]), uv);
}

layout(buffer_reference, std430) readonly buffer TEMPBUFFERSTRUCT {
    vec3 color;
};

layout(push_constant) uniform PC {
    uint srcIndex;
    uint samplerIndex;
    TEMPBUFFERSTRUCT temp;
} pc;

layout(location = 0) in  vec2 vUV;
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = SampleBindless(pc.srcIndex, pc.samplerIndex, vUV);
    // FragColor = vec4(pc.temp.color, 1.0);
}