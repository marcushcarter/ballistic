#pragma once
#include <glad/glad.h>

namespace gl {

    class Buffer {
    public:
        Buffer() = default;
        ~Buffer() { destroy(); }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept { *this = std::move(other); }
        Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                m_size = other.m_size;
                m_target = other.m_target;
                other.m_id = 0;
                other.m_size = 0;
                other.m_target = 0;
            }
            return *this;
        }

        void create(GLenum target);
        void destroy();

        bool valid() const { return m_id != 0 && glIsBuffer(m_id); }
        GLuint get() const { return m_id; }

        void bind() const;
        void unbind() const;

        void bindBase(GLenum target, GLuint index) const;
        void bindRange(GLenum target, GLuint index, GLintptr offset, GLsizeiptr size) const;

        void data(GLsizeiptr size, const void* data, GLenum usage);
        void update(GLintptr offset, GLsizeiptr size, const void* data);
        void storage(GLsizeiptr size, const void* data, GLbitfield flags);

        void clear(GLenum internalFormat, GLenum format, GLenum type, const void* data);
        void clearRange(GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
        void copy(const Buffer& src, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

        void* map(GLenum access);
        void* mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access);
        void flush(GLintptr offset, GLsizeiptr length);
        void unmap();
        
        template<typename T>
        T* mapTyped(GLenum access) { return static_cast<T*>(map(access)); }

        void getParameter(GLenum pname, GLint* params);
        void getPointer(GLenum target, GLenum pname, void** params);

        void label(const char* name);
        
        GLsizeiptr size() const { return m_size; }
        GLenum target() const { return m_target; }

    private:
        GLuint m_id{0};
        GLsizeiptr m_size{0};
        GLenum m_target{0};
    };

}