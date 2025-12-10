#include "Platform/OpenGL/ComputeTexture.h"

ComputeTexture::ComputeTexture(int width, int height, GLenum internalFormat) {
    m_Width = width;
    m_Height = height;
    m_InternalFormat = internalFormat;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

ComputeTexture::~ComputeTexture() {
    if (m_RendererID) {
        glDeleteTextures(1, &m_RendererID);
        m_RendererID = 0;
    }
}

void ComputeTexture::bind(GLuint unit) const {
    if (s_Bindings[unit] != m_RendererID) {
        glBindImageTexture(unit, m_RendererID, 0, GL_FALSE, 0, GL_READ_WRITE, m_InternalFormat);
        s_Bindings[unit] = m_RendererID;
    }
}

void ComputeTexture::unbind(GLuint unit) const {
    auto it = s_Bindings.find(unit);
    if (it != s_Bindings.end() && it->second != 0) {
        glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_WRITE, m_InternalFormat);
        s_Bindings.erase(it);
    }
}
