#pragma once
#include <glad/glad.h>

namespace gl {

    class VertexArray {
    public:
        VertexArray() = default;
        ~VertexArray() { destroy(); }

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        VertexArray(VertexArray&& other) noexcept { *this = std::move(other); }
        VertexArray& operator=(VertexArray&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        void create();
        void destroy();

        bool valid() const { return m_id != 0 && glIsVertexArray(m_id); }
        GLuint get() const { return m_id; }

        void bind() const;
        void unbind() const;

        void vertexBuffer(GLuint bindingIndex, GLuint buffer, GLintptr offset = 0, GLsizei stride = 0);
        void indexBuffer(GLuint buffer);

        void vertexAttrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLuint bindingIndex, GLuint relativeOffset = 0);
        void attribDivisor(GLuint index, GLuint divisor);
        void enableAttrib(GLuint index);
        void disableAttrib(GLuint index);

        void drawArrays(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount = 1) const;
        void drawElements(GLenum mode, GLsizei count, const void* indices = nullptr, GLsizei instanceCount = 1) const;

        void label(const char* name);

    private:
        GLuint m_id{0};
        GLenum m_indexType{GL_UNSIGNED_INT};
    };

}