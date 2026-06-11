#ifndef BINDLESS_GLSL
#define BINDLESS_GLSL

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform texture2D uTextures[];
layout(set = 0, binding = 1) uniform image2D uStorageImages[];
layout(set = 0, binding = 2) uniform sampler uSamplers[];

vec4 SampleBindless(uint texIndex, uint sampIndex, vec2 uv) {
    return texture(sampler2D(uTextures[nonuniformEXT(texIndex)], uSamplers[nonuniformEXT(sampIndex)]), uv);
}

#endif