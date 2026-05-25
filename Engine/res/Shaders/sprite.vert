#version 450
layout(push_constant) uniform PC { float x, y, w, h; } pc;
layout(location = 0) out vec2 vUv;
void main() {
    vec2 verts[6] = vec2[](
        vec2(0,0), vec2(1,0), vec2(1,1),
        vec2(0,0), vec2(1,1), vec2(0,1)
    );
    vec2 v = verts[gl_VertexIndex];
    vUv = v;
    gl_Position = vec4((pc.x + v.x * pc.w) * 2.0 - 1.0, (pc.y + v.y * pc.h) * 2.0 - 1.0, 0.0, 1.0);
}