#version 460 core

uniform vec4 uColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor.rgb, uColor.a);
}