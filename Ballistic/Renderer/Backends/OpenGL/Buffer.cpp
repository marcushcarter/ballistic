#include "Buffer.h"
#include "State.h"
#include <utility>

namespace gl {

    void Buffer::create(GLenum target) {
        if (m_id) return;
        m_target = target;
        if (GLAD_GL_VERSION_4_5)
            glCreateBuffers(1, &m_id);
        else
            glGenBuffers(1, &m_id);
    }

    void Buffer::destroy() {
        if (!m_id) return;
        glDeleteBuffers(1, &m_id);
        m_id = 0;
        m_size = 0;
        m_target = 0;
    }

    void Buffer::bind() const {
        State::bindBuffer(m_target, m_id);
    }

    void Buffer::unbind() const {
        State::bindBuffer(m_target, 0);
    }

    void Buffer::bindBase(GLenum target, GLuint index) const {
        glBindBufferBase(target, index, m_id);
    }

    void Buffer::bindRange(GLenum target, GLuint index, GLintptr offset, GLsizeiptr size) const {
        glBindBufferRange(target, index, m_id, offset, size);
    }

    void Buffer::data(GLsizeiptr size, const void* data, GLenum usage) {
        m_size = size;
        if (GLAD_GL_VERSION_4_5)
            glNamedBufferData(m_id, size, data, usage);
        else {
            bind();
            glBufferData(m_target, size, data, usage);
        }
    }

    void Buffer::update(GLintptr offset, GLsizeiptr size, const void* data) {
        if (GLAD_GL_VERSION_4_5)
            glNamedBufferSubData(m_id, offset, size, data);
        else {
            bind();
            glBufferSubData(m_target, offset, size, data);
        }
    }

    void Buffer::storage(GLsizeiptr size, const void* data, GLbitfield flags) {
        if (GLAD_GL_VERSION_4_5)
            glNamedBufferStorage(m_id, size, data, flags);
        else {
            bind();
            glBufferStorage(m_target, size, data, flags);
        }
    }

    void Buffer::clear(GLenum internalFormat, GLenum format, GLenum type, const void* data) {
        if (GLAD_GL_VERSION_4_5)
            glClearNamedBufferData(m_id, internalFormat, format, type, data);
        else {
            bind();
            glClearBufferData(m_target, internalFormat, format, type, data);
        }
    }

    void Buffer::clearRange(GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
        if (GLAD_GL_VERSION_4_5)
            glClearNamedBufferSubData(m_id, internalFormat, offset, size, format, type, data);
        else {
            bind();
            glClearBufferSubData(m_target, internalFormat, offset, size, format, type, data);
        }
    }
    
    void Buffer::copy(const Buffer& src, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
        if (GLAD_GL_VERSION_4_5)
            glCopyNamedBufferSubData(src.m_id, m_id, readOffset, writeOffset, size);
        else
            glCopyBufferSubData(src.m_target, m_target, readOffset, writeOffset, size);
    }

    void* Buffer::map(GLenum access) {
        if (GLAD_GL_VERSION_4_5) 
            return glMapNamedBuffer(m_id, access);
        bind();
        return glMapBuffer(m_target, access);
    }

    void* Buffer::mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) {
        if (GLAD_GL_VERSION_4_5)
            return glMapNamedBufferRange(m_id, offset, length, access);
        bind();
        return glMapBufferRange(m_target, offset, length, access);
    }

    void Buffer::flush(GLintptr offset, GLsizeiptr length) {
        if (GLAD_GL_VERSION_4_5)
            glFlushMappedNamedBufferRange(m_id, offset, length);
        else {
            bind();
            glFlushMappedBufferRange(m_target, offset, length);
        }
    }

    void Buffer::unmap() {
        if (GLAD_GL_VERSION_4_5)
            glUnmapNamedBuffer(m_id);
        else {
            bind();
            glUnmapBuffer(m_target);
        }
    }

    void Buffer::getParameter(GLenum pname, GLint* params) {
        if (GLAD_GL_VERSION_4_5)
            glGetNamedBufferParameteriv(m_id, pname, params);
        else {
            bind();
            glGetBufferParameteriv(m_target, pname, params);
        }
    }

    void Buffer::getPointer(GLenum target, GLenum pname, void** params) {
        if (GLAD_GL_VERSION_4_5)
            glGetNamedBufferPointerv(m_id, pname, params);
        else {
            bind();
            glGetBufferPointerv(m_target, pname, params);
        }
    }

    void Buffer::label(const char* name) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(GL_BUFFER, m_id, -1, name);
    }

}