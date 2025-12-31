#include "Root/MeshManager/MeshManager.h"
#include "Root/LogManager/Log.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ballistic
{
    bool MeshManager::Init() {
        LogDebug("Mesh manager initialized");
        return true;
    }

    void MeshManager::Shutdown() {

    }

    GUID MeshManager::AddMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<uint32_t>& inds) {
        MeshMetadata meta;
        meta.guid = GUID::Create();
        meta.name = name;
        meta.vertexOffset = m_vertices.size();
        meta.vertexCount = verts.size();
        meta.indexOffset = m_indices.size();
        meta.indexCount = inds.size();

        m_vertices.insert(m_vertices.end(), verts.begin(), verts.end());
        m_indices.insert(m_indices.end(), inds.begin(), inds.end());
        m_metadata.push_back(meta);

        size_t idx = m_metadata.size() - 1;
        guidToIndex[meta.guid] = idx;
        m_metadataDirty.push_back(idx);

        LogInfo("Mesh '", name, "' Loaded succesfully");
        return meta.guid;
    }
    
    GUID MeshManager::LoadMesh(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            LogError("Mesh file does not exist: ", path.string());
            return {};
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path.string(),
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices
        );

        if (!scene || !scene->HasMeshes()) {
            LogError("Assimp failed to load mesh: ", path.string());
            return {};
        }

         aiMesh* mesh = scene->mMeshes[0];

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        vertices.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 3);

        // Vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex v{};
            v.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };

            if (mesh->HasNormals()) {
                v.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }

            if (mesh->HasTextureCoords(0)) {
                v.uv = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            vertices.push_back(v);
        }

        // Indices
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];

            // Triangulate guarantees exactly 3
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        std::string name = mesh->mName.length > 0
            ? mesh->mName.C_Str()
            : path.filename().string();

        return AddMesh(name, vertices, indices);
    }

    void MeshManager::Clear() {
        m_vertices.clear();
        m_indices.clear();
        m_metadata.clear();
        m_metadataDirty.clear();
        guidToIndex.clear();
    }

    void MeshManager::MarkDirty(GUID guid) {
        auto it = guidToIndex.find(guid);
        if (it != guidToIndex.end()) {
            size_t idx = it->second;
            m_metadataDirty.push_back(idx);
        }
    }

    GUID MeshManager::GetMeshGUIDByName(const std::string& name) const {
        for (const auto& meta : m_metadata)
        {
            if (meta.name == name)
                return meta.guid;
        }

        return {};
    }

    const MeshMetadata* MeshManager::GetMeshMetadata(GUID guid) const {
        auto it = guidToIndex.find(guid);
        if (it != guidToIndex.end())
            return &m_metadata[it->second];
        return nullptr;
    }
    
} // namespace ballistic
