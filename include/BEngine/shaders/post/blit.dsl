@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec2 aTex;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTex;
}

@end

@fs fragment_shader

#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D screenTexture;
void main() {
    FragColor = texture(screenTexture, TexCoord);
}

@end

@program __blit vertex_shader fragment_shader