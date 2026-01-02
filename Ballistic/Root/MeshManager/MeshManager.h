#pragma once
#include "bepch.h"
#include "Root/MeshManager/Vertex.h"
#include "Root/MeshManager/MeshMetaData.h"

namespace ballistic
{
    class MeshManager
    {
    public:
        MeshManager() = default;
        ~MeshManager() { Shutdown(); }

        bool Init();
        void Shutdown();

        GUID AddMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<uint32_t>& inds);
        GUID LoadMesh(const std::filesystem::path& path, bool serialize = true);
        void Clear();

        void SerializeMesh(const MeshMetadata& meta, const std::filesystem::path& sourceMeshPath);
        void DeserializeMeshes();

        GUID GetMeshGUIDByName(const std::string& name) const;
        const MeshMetadata* GetMeshMetadata(GUID guid) const;

        GLuint GetVAO() const { return m_vao; }
        size_t GetVertexCount() const { return m_vertices.size(); }
        size_t GetIndexCount() const { return m_indices.size(); }
        const std::vector<Vertex>& GetVertexBuffer() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndexBuffer() const { return m_indices; }
        
        const std::vector<MeshMetadata>& GetAllMetadata() const { return m_metadata; }

    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<MeshMetadata> m_metadata;
        std::unordered_map<GUID, size_t> guidToIndex;
        
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;
        size_t m_vertexCapacityBytes = 0;
        size_t m_indexCapacityBytes  = 0;

        void EnsureBuffers();
    };
    
} // namespace ballistic
