#include "VertexArray.h"
#include "State.h"
#include <utility>
#include <vector>

namespace gl {
    
    void VertexArray::create() {
        if (m_id) return;
        if (GLAD_GL_VERSION_4_5)
            glCreateVertexArrays(1, &m_id);
        else
            glGenVertexArrays(1, &m_id);
    }

    void VertexArray::destroy() {
        if (!m_id) return;
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }

    void VertexArray::bind() const {
        State::bindVertexArray(m_id);
    }
    
    void VertexArray::unbind() const {
        State::bindVertexArray(0);
    }

    void VertexArray::vertexBuffer(GLuint bindingIndex, GLuint buffer, GLintptr offset, GLsizei stride) {
        if (GLAD_GL_VERSION_4_5)
            glVertexArrayVertexBuffer(m_id, bindingIndex, buffer, offset, stride);
        else {
            bind();
            glBindVertexBuffer(bindingIndex, buffer, offset, stride);
        }
    }

    void VertexArray::indexBuffer(GLuint buffer) {
        if (GLAD_GL_VERSION_4_5)
            glVertexArrayElementBuffer(m_id, buffer);
        else {
            bind();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        }
    }

    void VertexArray::vertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLuint bindingIndex, GLuint relativeOffset) {
        if (GLAD_GL_VERSION_4_5) {
            glEnableVertexArrayAttrib(m_id, index);
            glVertexArrayAttribFormat(m_id, index, size, type, normalized, relativeOffset);
            glVertexArrayAttribBinding(m_id, index, bindingIndex);
        } else {
            bind();
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, size, type, normalized, 0, (void*)(uintptr_t)relativeOffset);
        }
    }

    void VertexArray::attribDivisor(GLuint index, GLuint divisor) {
        if (GLAD_GL_VERSION_4_5)
            glVertexArrayBindingDivisor(m_id, index, divisor);
        else {
            bind();
            glVertexBindingDivisor(index, divisor);
        }
    }

    void VertexArray::enableAttrib(GLuint index) {
        if (GLAD_GL_VERSION_4_5)
            glEnableVertexArrayAttrib(m_id, index);
        else {
            bind();
            glEnableVertexAttribArray(index);
        }
    }

    void VertexArray::disableAttrib(GLuint index) {
        if (GLAD_GL_VERSION_4_5)
            glDisableVertexArrayAttrib(m_id, index);
        else {
            bind();
            glDisableVertexAttribArray(index);
        }
    }

    void VertexArray::drawArrays(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) const {
        bind();
        if (instanceCount > 1)
            glDrawArraysInstanced(mode, first, count, instanceCount);
        else
            glDrawArrays(mode, first, count);
    }

    void VertexArray::drawElements(GLenum mode, GLsizei count, const void* indices, GLsizei instanceCount) const {
        bind();
        if (instanceCount > 1)
            glDrawElementsInstanced(mode, count, m_indexType, indices, instanceCount);
        else
            glDrawElements(mode, count, m_indexType, indices);
    }

    void VertexArray::label(const char* name) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(GL_VERTEX_ARRAY, m_id, -1, name);
    }
}