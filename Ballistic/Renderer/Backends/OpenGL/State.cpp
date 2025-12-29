#include "State.h"
#include <utility>

namespace gl {

    void State::bindBuffer(GLenum target, GLuint id) {
        if (boundBuffers[target] != id) {
            glBindBuffer(target, id);
            boundBuffers[target] = id;
        }
    }

    void State::bindBufferBase(GLenum target, GLuint index, GLuint id) {
        auto key = std::make_pair(target, index);
        if (boundBases[key] != id) {
            glBindBufferBase(target, index, id);
            boundBases[key] = id;
        }
    }

    void State::bindBufferRange(GLenum target, GLuint index, GLuint id, GLintptr offset, GLsizeiptr size) {
        auto key = std::make_pair(target, index);
        if (boundBases[key] != id) {
            glBindBufferRange(target, index, id, offset, size);
            boundBases[key] = id;
        }
    }
    
    void State::bindVertexArray(GLuint id) {
        if (boundVertexArray != id) {
            glBindVertexArray(id);
            boundVertexArray = id;
        }
    }

    void State::bindShader(GLuint id) {
        if (boundShader != id) {
            glUseProgram(id);
            boundShader = id;
        }
    }

    void State::bindTexture(GLuint unit, GLenum target, GLuint id) {
        activeTexture(unit);

        if (GLAD_GL_VERSION_4_5 || GLAD_GL_ARB_direct_state_access) {
            if (boundTextureUnits[unit] != id) {
                glBindTextureUnit(unit, id);
                boundTextureUnits[unit] = id;
            }
        } else {
            auto key = std::make_pair(target, unit);
            if (boundTextures[key] != id) {
                glBindTexture(target, id);
                boundTextures[key] = id;
            }
        }
    }

    void State::activeTexture(GLuint unit) {
        if (activeTexUnit != unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            activeTexUnit = unit;
        }
    }

    void State::bindRenderbuffer(GLuint id) {
        if (boundRenderbuffer != id) {
            glBindRenderbuffer(GL_RENDERBUFFER, id);
            boundRenderbuffer = id;
        }
    }

    void State::bindFramebuffer(GLuint id, GLenum target) {
        GLuint* slot = nullptr;

        switch (target) {
            case GL_FRAMEBUFFER:
                slot = &boundFramebuffer;
                break;
            case GL_DRAW_FRAMEBUFFER:
                slot = &boundDrawFramebuffer;
                break;
            case GL_READ_FRAMEBUFFER:
                slot = &boundReadFramebuffer;
                break;
            default:
                glBindFramebuffer(target, id);
                return;
        }

        if (*slot != id) {
            glBindFramebuffer(target, id);
            *slot = id;
        }
    }

    void State::reset() {
        boundBuffers.clear();
        boundBases.clear();
        boundTextures.clear();
        boundTextureUnits.clear();

        boundVertexArray = 0;
        boundShader = 0;
        boundRenderbuffer = 0;

        boundFramebuffer = 0;
        boundDrawFramebuffer = 0;
        boundReadFramebuffer = 0;

        activeTexUnit = 0;
    }
}