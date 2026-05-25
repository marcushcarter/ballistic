#version 450
#extension GL_EXT_demote_to_helper_invocation : disable
layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 0) uniform sampler2D image;
void main() {
    vec4 color = texture(image, vUv);
    if (color.a < 0.1) discard;
    FragColor = color;
}