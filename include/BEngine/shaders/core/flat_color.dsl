@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}

@end

@fs fragment_shader

#version 460 core

uniform vec4 uColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor.rgb, clamp(uColor.a, 0.0, 1.0));
}

@end

@program __flat_color vertex_shader fragment_shader