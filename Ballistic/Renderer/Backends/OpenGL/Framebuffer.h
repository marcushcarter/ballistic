#pragma once
#include <glad/glad.h>
#include "Renderbuffer.h"
#include "Texture2D.h"
#include <vector>

namespace gl {

    class Framebuffer {
    public:
        Framebuffer() = default;
        ~Framebuffer() { destroy(); }

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        Framebuffer(Framebuffer&& other) noexcept { *this = std::move(other); }
        Framebuffer& operator=(Framebuffer&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        void create();
        void destroy();

        bool valid() const { return m_id != 0 && glIsFramebuffer(m_id); }
        bool complete() const { bind(); return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }
        GLuint get() const { return m_id; }

        void bind(GLenum target = GL_FRAMEBUFFER) const;
        void unbind(GLenum target = GL_FRAMEBUFFER) const;

        void attachColor(GLuint slot, Texture2D& texture);
        void attachDepth(Renderbuffer& rbo);
        void attachDepth(Texture2D& texture);

        void label(const char* name);

        Texture2D* getColorAttachment(GLuint slot) const {
            if (slot >= m_colorAttachments.size()) return nullptr;
            return m_colorAttachments[slot];
        }
        
    private:
        GLuint m_id = 0;
        std::vector<Texture2D*> m_colorAttachments;
        Texture2D* m_depthTexture = nullptr;
        Renderbuffer* m_depthRBO = nullptr;
    };

}
