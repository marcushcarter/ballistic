#include "Framebuffer.h"
#include "State.h"

namespace gl {

    void Framebuffer::create() {
        if (m_id) return;
        if (GLAD_GL_VERSION_4_5)
            glCreateFramebuffers(1, &m_id);
        else
            glGenFramebuffers(1, &m_id);
    }

    void Framebuffer::destroy() {
        if (!m_id) return;
        glDeleteFramebuffers(1, &m_id);
        m_id = 0;
        m_colorAttachments.clear();
        m_depthRBO = nullptr;
        m_depthTexture = nullptr;
    }

    void Framebuffer::bind(GLenum target) const {
        State::bindFramebuffer(m_id, target);
    }
    
    void Framebuffer::unbind(GLenum target) const {
        State::bindFramebuffer(0, target);
    }

    void Framebuffer::attachColor(GLuint slot, Texture2D& texture) {
        create();
        m_colorAttachments.resize(std::max<size_t>(slot + 1, m_colorAttachments.size()));
        m_colorAttachments[slot] = &texture;
        if (GLAD_GL_VERSION_4_5)
            glNamedFramebufferTexture(m_id, GL_COLOR_ATTACHMENT0 + slot, texture.get(), 0);
        else {
            bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, texture.get(), 0);
        }
    }

    void Framebuffer::attachDepth(Renderbuffer& rbo) {
        m_depthRBO = &rbo;
        m_depthTexture = nullptr;
        rbo.attachToFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT);
    }

    void Framebuffer::attachDepth(Texture2D& texture) {
        m_depthTexture = &texture;
        m_depthRBO = nullptr;
        if (GLAD_GL_VERSION_4_5)
            glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, texture.get(), 0);
        else {
            bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.get(), 0);
        }
    }

    void Framebuffer::label(const char* name) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(GL_FRAMEBUFFER, m_id, -1, name);
    }

}
