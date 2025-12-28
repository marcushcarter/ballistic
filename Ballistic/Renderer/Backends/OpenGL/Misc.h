#pragma once
#include <glad/glad.h>

namespace gl {
    
    inline void ClearColor(float r, float g, float b, float a = 1.0f) {
        glClearColor(r, g, b, a);
    }

    inline void Clear(GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) {
        glClear(mask);
    }

    inline void Viewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
    }

    inline void Scissor(int x, int y, int w, int h) {
        glScissor(x, y, w, h);
    }

    inline void DepthFunc(GLenum func) {
        glDepthFunc(func);
    }

    inline void DepthMask(bool write) {
        glDepthMask(write ? GL_TRUE : GL_FALSE);
    }

    inline void BlendFunc(GLenum src, GLenum dst) {
        glBlendFunc(src, dst);
    }

    inline void CullFace(GLenum face) {
        glCullFace(face);
    }

    inline void FrontFace(GLenum winding) {
        glFrontFace(winding);
    }

    inline void PolygonMode(GLenum mode) {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
    }

    inline void ColorMask(bool r, bool g, bool b, bool a = 1.0f) {
        glColorMask(r, g, b, a);
    }
}