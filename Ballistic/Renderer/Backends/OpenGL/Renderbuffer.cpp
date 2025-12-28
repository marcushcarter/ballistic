#include "Renderbuffer.h"
#include "State.h"
#include <utility>
#include <vector>

namespace gl {

    void Renderbuffer::create() {
        if (m_id) return;
        if (GLAD_GL_VERSION_4_5)
            glCreateRenderbuffers(1, &m_id);
        else
            glGenRenderbuffers(1, &m_id);
    }

    void Renderbuffer::destroy() {
        if (!m_id) return;
        glDeleteRenderbuffers(1, &m_id);
        m_id = 0;
        m_width = 0;
        m_height = 0;
        m_samples = 0;
        m_internalFormat = 0;
    }

    void Renderbuffer::bind() const {
        State::bindRenderbuffer(m_id);
    }

    void Renderbuffer::unbind() const {
        State::bindRenderbuffer(0);
    }

    void Renderbuffer::storage(GLenum internalFormat, GLsizei width, GLsizei height) {
        m_internalFormat = internalFormat;
        m_width = width;
        m_height = height;
        m_samples = 0;

        if (GLAD_GL_VERSION_4_5)
            glNamedRenderbufferStorage(m_id, internalFormat, width, height);
        else {
            bind();
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
        }
    }

    void Renderbuffer::storageMultisample(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height) {
        m_internalFormat = internalFormat;
        m_width = width;
        m_height = height;
        m_samples = samples;

        if (GLAD_GL_VERSION_4_5)
            glNamedRenderbufferStorageMultisample(m_id, samples, internalFormat, width, height);
        else {
            bind();
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
        }
    }

    void Renderbuffer::attachToFramebuffer(GLenum target, GLenum attachment) const {
        glFramebufferRenderbuffer(target, attachment, GL_RENDERBUFFER, m_id);
    }

    void Renderbuffer::resize(GLsizei width, GLsizei height) {
        if (!m_id) return;

        if (m_samples > 0)
            storageMultisample(m_samples, m_internalFormat, width, height);
        else
            storage(m_internalFormat, width, height);
    }

    void Renderbuffer::label(const char* name) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(GL_RENDERBUFFER, m_id, -1, name);
    }
}