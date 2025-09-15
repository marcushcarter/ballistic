@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTex;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 TexCoord;
out vec3 Normal;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    TexCoord = aTex;
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
}

@end

@fs fragment_shader

#version 460 core

in vec2 TexCoord;
in vec3 Normal;
out vec4 FragColor;

uniform sampler2D diffuse0;

void main() {
    vec3 texColor = texture(-diffuse0, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    vec3 finalColor = vec3(0.05);

    vec3 color = texColor * finalColor;

    FragColor = vec4(color.r, color.g, color.b, 1);

}

@end

@program scene2 vertex_shader fragment_shader