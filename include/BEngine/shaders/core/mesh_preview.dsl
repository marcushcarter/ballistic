@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec2 aTex;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 TexCoord;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    TexCoord = aTex;
}

@end

@fs fragment_shader

#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D diffuseMap;

void main() {
    float colormix = 0.5 * TexCoord.x + 0.5 * TexCoord.y;
    FragColor = vec4(TexCoord, 1, 1.0);
}

@end

@program __mesh_preview vertex_shader fragment_shader