#include "Root/MeshManager/MeshManager.h"
#include "Root/LogManager/Log.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <nlohmann/json.hpp>

namespace ballistic
{
    bool MeshManager::Init() {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_vertexCapacityBytes  = 1000 * sizeof(Vertex);
        glBufferData(GL_ARRAY_BUFFER, m_vertexCapacityBytes , nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        m_indexCapacityBytes  = 3000 * sizeof(uint32_t);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCapacityBytes , nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

        glBindVertexArray(0);

        LogDebug("Mesh manager initialized");
        return true;
    }

	void MeshManager::EnsureBuffers() {
		size_t vertexRequiredBytes = m_vertices.size() * sizeof(Vertex);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		if (vertexRequiredBytes > m_vertexCapacityBytes) {
			m_vertexCapacityBytes = std::max(vertexRequiredBytes, m_vertexCapacityBytes * 2);
			glBufferData(GL_ARRAY_BUFFER, m_vertexCapacityBytes, m_vertices.data(), GL_DYNAMIC_DRAW);
			LogWarn("Vertex buffer resized - new size: ", m_vertexCapacityBytes);
		} else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertexRequiredBytes, m_vertices.data());
		}
        
		size_t indexRequiredBytes = m_indices.size() * sizeof(uint32_t);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

		if (indexRequiredBytes > m_indexCapacityBytes) {
			m_indexCapacityBytes = std::max(indexRequiredBytes, m_indexCapacityBytes * 2);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCapacityBytes, m_indices.data(), GL_DYNAMIC_DRAW);
			LogWarn("Vertex buffer resized - new size: ", m_indexCapacityBytes);
		} else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexRequiredBytes, m_indices.data());
		}
    }

    GUID MeshManager::AddMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<uint32_t>& inds) {
        size_t startVertex = m_vertices.size();
        size_t startIndex  = m_indices.size();

        m_vertices.insert(m_vertices.end(), verts.begin(), verts.end());
        m_indices.insert(m_indices.end(), inds.begin(), inds.end());

        EnsureBuffers();
        
        MeshMetadata meta;
        meta.guid = GUID::Create();
        meta.name = name;
        meta.vertexOffset = startVertex;
        meta.vertexCount = verts.size();
        meta.indexOffset = startIndex;
        meta.indexCount = inds.size();

        m_metadata.push_back(meta);
        guidToIndex[meta.guid] = m_metadata.size() - 1;

        LogInfo("Mesh '", name, "' Loaded succesfully");
        return meta.guid;
    }
    
    GUID MeshManager::LoadMesh(const std::filesystem::path& path, bool serialize) {
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

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        size_t vertexOffset = 0;

        for (uint32_t m = 0; m < scene->mNumMeshes; ++m) {
            aiMesh* mesh = scene->mMeshes[m];

            // Add vertices
            for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
                Vertex v{};
                v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                if (mesh->HasNormals())
                    v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                if (mesh->HasTextureCoords(0))
                    v.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                vertices.push_back(v);
            }

            // Add indices with offset
            for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
                const aiFace& face = mesh->mFaces[i];
                indices.push_back(face.mIndices[0] + vertexOffset);
                indices.push_back(face.mIndices[1] + vertexOffset);
                indices.push_back(face.mIndices[2] + vertexOffset);
            }

            vertexOffset = vertices.size();
        }

        std::string name = path.stem().string(); // Single name for all sub-meshes
        GUID guid = AddMesh(name, vertices, indices);

        if (serialize) {
            const MeshMetadata& meta = *GetMeshMetadata(guid);
            SerializeMesh(meta, path);
        }

        return guid;
    }

    void MeshManager::SerializeMesh(const MeshMetadata& meta, const std::filesystem::path& sourceMeshPath) {
        std::filesystem::path projectRoot = GetRoot()->GetProjectManager()->GetProjectRoot();

        std::filesystem::path destDir = projectRoot / "Resources" / "Meshes" / std::to_string(meta.guid.value);
        std::filesystem::create_directories(destDir);

        const std::filesystem::path sourceDir = sourceMeshPath.parent_path();

        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
            const auto& srcPath = entry.path();
            std::filesystem::path relativePath = std::filesystem::relative(srcPath, sourceDir);
            std::filesystem::path destPath = destDir / relativePath;

            if (entry.is_directory()) {
                std::filesystem::create_directories(destPath);
            } else if (entry.is_regular_file()) {
                std::filesystem::copy_file(
                    srcPath,
                    destPath,
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }

        LogInfo("Serialized mesh files for '", meta.name, "' to ", destDir.string());

        nlohmann::json j;
        j["guid"] = std::to_string(meta.guid.value);
        j["name"] = meta.name;
        j["source"] = std::filesystem::relative(destDir / sourceMeshPath.filename(), destDir).string();
        j["type"] = sourceMeshPath.extension().string();

        std::filesystem::path metaFile = destDir / "mesh.meta.json";
        std::ofstream out(metaFile);
        if (!out.is_open()) {
            LogError("Failed to write mesh metadata: ", metaFile.string());
            return;
        }
        out << j.dump(4);
        out.close();

        LogInfo("Wrote mesh meta: ", metaFile.string());
    }
    
    void MeshManager::DeserializeMeshes() {
        Clear();

        std::filesystem::path projectRoot = GetRoot()->GetProjectManager()->GetProjectRoot();
        std::filesystem::path meshesRoot = projectRoot / "Resources" / "Meshes";

        if (!std::filesystem::exists(meshesRoot)) {
            LogWarn("No meshes folder found at: ", meshesRoot.string());
            return;
        }

        // Loop through all subfolders (each GUID folder)
        for (const auto& entry : std::filesystem::directory_iterator(meshesRoot)) {
            if (!entry.is_directory())
                continue;

            std::filesystem::path guidFolder = entry.path();
            std::filesystem::path metaFile = guidFolder / "mesh.meta.json";

            if (!std::filesystem::exists(metaFile)) {
                LogWarn("Missing mesh.meta.json in folder: ", guidFolder.string());
                continue;
            }

            // Read metadata
            std::ifstream in(metaFile);
            if (!in.is_open()) {
                LogError("Failed to open mesh.meta.json: ", metaFile.string());
                continue;
            }

            nlohmann::json j;
            in >> j;
            in.close();

            // Reconstruct mesh path relative to GUID folder
            std::filesystem::path meshFile = guidFolder / j["source"].get<std::string>();

            if (!std::filesystem::exists(meshFile)) {
                LogError("Mesh file referenced in meta does not exist: ", meshFile.string());
                continue;
            }

            // Load the mesh
            GUID guid = LoadMesh(meshFile, false);

            // Optional: you could overwrite the generated GUID with the one in meta
            // but usually you just keep LoadMesh generated GUID
            LogInfo("Deserialized mesh '", j["name"].get<std::string>(), "' from ", meshFile.string());
        }

        LogInfo("Finished deserializing all meshes.");

    }

    void MeshManager::Clear() {
        m_vertices.clear();
        m_indices.clear();
        m_metadata.clear();
        guidToIndex.clear();
    }

    GUID MeshManager::GetMeshGUIDByName(const std::string& name) const {
        for (const auto& meta : m_metadata)
            if (meta.name == name) return meta.guid;
        return {};
    }

    const MeshMetadata* MeshManager::GetMeshMetadata(GUID guid) const {
        auto it = guidToIndex.find(guid);
        if (it != guidToIndex.end())
            return &m_metadata[it->second];
        return nullptr;
    }

    void MeshManager::Shutdown() {
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
        glDeleteVertexArrays(1, &m_vao);

        m_vertices.clear();
        m_indices.clear();
        m_metadata.clear();
        guidToIndex.clear();
    }
    
} // namespace ballistic
