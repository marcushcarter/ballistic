#include "Buffer.h"

#include <cstring>
#include <cstdio>

Buffer::Buffer(GLenum target) : m_Target(target) {
    glGenBuffers(1, &m_RendererID);
}

Buffer::~Buffer() {
    if (m_RendererID) {
        glDeleteBuffers(1, &m_RendererID);
        m_RendererID = 0;
    }
}

void Buffer::allocate(size_t size, GLenum usage) {
    bind();
    m_Usage = usage;
    m_Size = size;
    glBufferData(m_Target, m_Size, nullptr, m_Usage);
}

void Buffer::uploadData(const void* data, size_t dataSize, GLenum usage) {
    bind();
    m_Usage = usage;
    if (dataSize > m_Size) {
        glBufferData(m_Target, dataSize, data, m_Usage);
        m_Size = dataSize;
    } else {
        glBufferSubData(m_Target, 0, dataSize, data);
    }//
}


void Buffer::bind(GLuint unit, size_t offset, size_t range) const {
    auto it = s_BoundBuffers.find(m_Target);
    if (it == s_BoundBuffers.end() || it->second != m_RendererID) {
        glBindBuffer(m_Target, m_RendererID);
        s_BoundBuffers[m_Target] = m_RendererID;
    }

    if (unit != UINT32_MAX) {
        auto& map = s_BoundBases[m_Target];
        auto baseIt = map.find(unit);
        if (baseIt == map.end() || baseIt->second != m_RendererID) {
            if (range > 0) {
                glBindBufferRange(m_Target, unit, m_RendererID, offset, range);
            } else {
                glBindBufferBase(m_Target, unit, m_RendererID);
            }
            map[unit] = m_RendererID;
        }
    }
}

void Buffer::unbind(GLuint unit) const {
    auto it = s_BoundBuffers.find(m_Target);
    if (it != s_BoundBuffers.end() && it->second != 0) {
        glBindBuffer(m_Target, 0);
        s_BoundBuffers[m_Target] = 0;
    }

    if (unit != UINT32_MAX) {
        auto& map = s_BoundBases[m_Target];
        auto baseIt = map.find(unit);
        if (baseIt != map.end()) {
            glBindBufferBase(m_Target, unit, 0);
            map.erase(baseIt);
        }
    }
}

void* Buffer::map(size_t offset, size_t size, GLbitfield access) {
    bind();
    if (size == 0) size = m_Size;
    void* ptr = glMapBufferRange(m_Target, offset, size, access);
    if (!ptr) {
        // fprintf(stderr, "Error: failed to map buffer (ID %u)\n", m_RendererID);
    }
    return ptr;
}

void Buffer::unmap() {
    bind();
    if (!glUnmapBuffer(m_Target)) {
        // fprintf(stderr, "Warning: buffer data may have been corrupted (ID %u)\n", m_RendererID);
    }
}

template<typename T>
bool Buffer::read(T* cpuData, size_t count) {
    size_t totalSize = sizeof(T) * count;
    if (totalSize > m_Size) {
        // fprintf(stderr, "Error: readAll exceeds buffer size (ID %u)\n", m_RendererID);
        return false;
    }

    void* ptr = map(0, totalSize, GL_MAP_READ_BIT);
    if (!ptr) return false;

    memcpy(cpuData, ptr, totalSize);
    unmap();
    return true;
}

template<typename T>
bool Buffer::read(std::vector<T>& cpuVec) {
    size_t totalSize = m_Size;
    if (totalSize % sizeof(T) != 0) {
        fprintf(stderr, "Error: buffer size not divisible by type size (ID %u)\n", m_RendererID);
        return false;
    }

    size_t count = totalSize / sizeof(T);
    cpuVec.resize(count);

    void* ptr = map(0, totalSize, GL_MAP_READ_BIT);
    if (!ptr) return false;

    memcpy(cpuVec.data(), ptr, totalSize);
    unmap();
    return true;
}

void Buffer::setLabel(const char* name) const {
	glObjectLabel(GL_BUFFER, m_RendererID, -1, name);
}