@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTex;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    TexCoord = aTex;
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
}

@end

@fs fragment_shader

#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D diffuse0;

uniform int numLights;
struct Light {
    vec4 position;
    vec4 color;
    vec4 direction;
    mat4 shadowMatrices[2];
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
            vec3 lightDir = normalize(-lights[i].direction.xyz);
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
            float theta = dot(lightDir, normalize(-lights[i].direction.xyz));
            if (theta > lights[i].direction.w) {
                float distance = length(lights[i].position.xyz - FragPos);
                float attenuation = 1.0 / (distance * distance);
                diff = max(dot(norm, lightDir), 0.0) * attenuation;
            }
        }

        finalColor += lightColor * diff;
    }

    FragColor = vec4(texColor * finalColor, 1.0);
}

@end

@program __scene vertex_shader fragment_shader