#include "Texture2D.h"
#include "State.h"

namespace gl {
    
    void Texture2D::create(GLsizei width, GLsizei height, GLenum internalFormat, GLenum format, GLenum type, GLsizei levels) {
        if (m_id) return;

        if (GLAD_GL_VERSION_4_5) {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
            glTextureStorage2D(m_id, levels, internalFormat, width, height);
        } else {
            glGenTextures(1, &m_id);
            bind();
            glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, width, height);
        }

        m_width = width;
        m_height = height;
        m_internalFormat = internalFormat;
        m_format = format;
        m_type = type;
        m_levels = levels;
    }

    void Texture2D::destroy() {
        if (!m_id) return;
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }

    void Texture2D::bind(GLuint unit) const {
        State::bindTexture(unit, GL_TEXTURE_2D, m_id);
    }
    
    void Texture2D::unbind(GLuint unit) const {
        State::bindTexture(unit, GL_TEXTURE_2D, 0);
    }
    
    void Texture2D::bindImage(GLuint unit, GLenum access, GLint level) const {
        glBindImageTexture(unit, m_id, level, GL_FALSE, 0, access, m_internalFormat);
    }

    void Texture2D::setData(const void* data) const {
        if (GLAD_GL_VERSION_4_5) {
            glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, m_format, m_type, data);
        } else {
            bind();
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, m_type, data);
        }
    }

    void Texture2D::getData(void* data) const {
        bind();
        glGetTexImage(GL_TEXTURE_2D, 0, m_format, m_type, data);
    }

    void Texture2D::resize(GLsizei width, GLsizei height) {
        if (!m_id) return;

        if (width == m_width && height == m_height)
            return;

        m_width = width;
        m_height = height;

        if (GLAD_GL_VERSION_4_5) {
            glTextureStorage2D(m_id, m_levels, m_internalFormat, width, height);
        } else {
            bind();
            glTexStorage2D(GL_TEXTURE_2D, m_levels, m_internalFormat, width, height);
        }
    }

    void Texture2D::generateMipmaps() const {
        if (GLAD_GL_VERSION_4_5) {
            glGenerateTextureMipmap(m_id);
        } else {
            bind();
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    void Texture2D::setParameters(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT) const {
        if (GLAD_GL_VERSION_4_5) {
            glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, minFilter);
            glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, magFilter);
            glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrapS);
            glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrapT);
        } else {
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        }
    }

    void Texture2D::label(const char* name) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(GL_TEXTURE, m_id, -1, name);
    }
}