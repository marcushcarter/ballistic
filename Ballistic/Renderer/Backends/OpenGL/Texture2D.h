#pragma once
#include <glad/glad.h>

namespace gl {

    class Texture2D {
    public:
        Texture2D() = default;
        ~Texture2D() { destroy(); }

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D(Texture2D&& other) noexcept { *this = std::move(other); }
        Texture2D& operator=(Texture2D&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        void create(GLsizei width, GLsizei height, GLenum internalFormat, GLenum format, GLenum type, GLsizei levels = 1);
        void destroy();
        
        bool valid() const { return m_id != 0 && glIsTexture(m_id); }
        GLuint get() const { return m_id; }

        void bind(GLuint unit = 0) const;
        void unbind(GLuint unit = 0) const;
        void bindImage(GLuint unit, GLenum access = GL_READ_WRITE, GLint level = 0) const;

        void setData(const void* data) const;
        void getData(void* data) const;
        
        void resize(GLsizei width, GLsizei height);
        void generateMipmaps() const;

        void setParameters(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT) const;

        void label(const char* name);

        GLsizei width() const { return m_width; }
        GLsizei height() const { return m_height; }

    private:
        GLuint m_id{0};
        GLsizei m_width{0}, m_height{0};
        GLsizei m_levels{1};
        GLenum m_internalFormat{0}, m_format{0}, m_type{0}; 
    };

}