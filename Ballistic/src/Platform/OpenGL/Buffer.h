#pragma once

#include "lrpch.h"
#include <glad/glad.h>

class Buffer {
public:    
    Buffer(GLenum target);
    ~Buffer();

    void allocate(size_t size, GLenum usage = GL_DYNAMIC_DRAW);
    void uploadData(const void* data, size_t dataSize, GLenum usage = GL_DYNAMIC_DRAW);

    void bind(GLuint unit = UINT32_MAX, size_t offset = 0, size_t range = 0) const;
    void unbind(GLuint unit = UINT32_MAX) const;

    void* map(size_t offset = 0, size_t size = 0, GLbitfield access = GL_MAP_WRITE_BIT);
    void unmap();

    template<typename T>
    bool read(T* cpuData, size_t count);

    template<typename T>
    bool read(std::vector<T>& cpuVec);
    
    void setLabel(const char* name) const;

    GLuint m_RendererID;
    size_t m_Size;
    GLenum m_Target = GL_ARRAY_BUFFER;
    GLenum m_Usage = GL_DYNAMIC_DRAW;

private:
    static inline std::unordered_map<GLenum, GLuint> s_BoundBuffers;
    static inline std::unordered_map<GLenum, std::unordered_map<GLuint, GLuint>> s_BoundBases;
};
