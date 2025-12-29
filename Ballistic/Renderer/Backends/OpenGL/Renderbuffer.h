#pragma once
#include <glad/glad.h>

namespace gl {

    class Renderbuffer {
    public:
        Renderbuffer() = default;
        ~Renderbuffer() { destroy(); }

        Renderbuffer(const Renderbuffer&) = delete;
        Renderbuffer& operator=(const Renderbuffer&) = delete;

        Renderbuffer(Renderbuffer&& other) noexcept { *this = std::move(other); }
        Renderbuffer& operator=(Renderbuffer&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                m_width = other.m_width;
                m_height = other.m_height;
                m_samples = other.m_samples;
                m_internalFormat = other.m_internalFormat;
                other.m_id = 0;
                other.m_width = 0;
                other.m_height = 0;
                other.m_samples = 0;
                other.m_internalFormat = 0;
            }
            return *this;
        }

        void create();
        void destroy();

        bool valid() const { return m_id != 0 && glIsRenderbuffer(m_id); }
        GLuint get() const { return m_id; }

        void bind() const;
        void unbind() const;

        void storage(GLenum internalFormat, GLsizei width, GLsizei height);
        void storageMultisample(GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);

        void attachToFramebuffer(GLenum target, GLenum attachment) const;
        void resize(GLsizei width, GLsizei height);

        void label(const char* name);

        GLsizei width() const { return m_width; }
        GLsizei height() const { return m_height; }
        GLsizei samples() const { return m_samples; }
        GLenum internalFormat() const { return m_internalFormat; }

    private:
        GLuint m_id = 0;
        GLsizei m_width = 0;
        GLsizei m_height = 0;
        GLsizei m_samples = 0;
        GLenum m_internalFormat = 0;
    };

}