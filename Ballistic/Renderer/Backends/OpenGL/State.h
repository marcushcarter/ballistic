#pragma once
#include <glad/glad.h>
#include <utility>

namespace gl {

    struct pair_hash {
        size_t operator()(const std::pair<GLenum, GLuint>& p) const noexcept {
            return std::hash<GLenum>{}(p.first) ^ (std::hash<GLuint>{}(p.second) << 1);
        }
    };

    class State {
    public:
        static void bindBuffer(GLenum target, GLuint id);
        static void bindBufferBase(GLenum target, GLuint index, GLuint id);
        static void bindBufferRange(GLenum target, GLuint index, GLuint id, GLintptr offset, GLsizeiptr size);
        
        static void bindVertexArray(GLuint id);

        static void bindShader(GLuint id);

        static void bindTexture(GLuint unit, GLenum target, GLuint id);
        static void activeTexture(GLuint unit);

        static void bindRenderbuffer(GLuint id);
        static void bindFramebuffer(GLuint id, GLenum target = GL_FRAMEBUFFER);

        static void reset();
    
    private:
        static inline std::unordered_map<GLenum, GLuint> boundBuffers;
        static inline std::unordered_map<std::pair<GLenum, GLuint>, GLuint, pair_hash> boundBases;

        static inline GLuint boundVertexArray = 0;
        static inline GLuint boundShader = 0;
        static inline GLuint boundRenderbuffer = 0;

        static inline GLuint boundFramebuffer = 0;
        static inline GLuint boundDrawFramebuffer = 0;
        static inline GLuint boundReadFramebuffer = 0;

        static inline GLuint activeTexUnit = 0;
        static inline std::unordered_map<std::pair<GLenum, GLuint>, GLuint, pair_hash> boundTextures;
        static inline std::unordered_map<GLuint, GLuint> boundTextureUnits;
    };
}