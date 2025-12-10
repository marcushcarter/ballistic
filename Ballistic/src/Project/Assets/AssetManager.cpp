#include "Project/Assets/AssetManager.h"

#include <../Thirdparty/tinyobjloader/tiny_obj_loader.h>

namespace Ballistic {

	int AssetManager::AddRtxMaterial(const std::string& name, const RtxMaterial& mat) {
	    int id = (int)rtxMaterials.size();
	    rtxMaterials.push_back(mat);

	    materialLookup[name] = id;
	    materialsChanged = true;

	    return id;
	}

	RtxMaterial& AssetManager::GetRtxMaterial(const std::string& name) {
		return rtxMaterials[ materialLookup[name] ];
	}

	int AssetManager::AddRtxMesh(const std::string& name, const std::string& path) {    
	    tinyobj::attrib_t attrib;
	    std::vector<tinyobj::shape_t> shapes;
	    std::vector<tinyobj::material_t> materials;
	    std::string warn, err;

	    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	    if (!warn.empty()) printf("TinyObjLoader warning: %s\n", warn.c_str());
	    if (!err.empty())  printf("TinyObjLoader error: %s\n", err.c_str());
	    if (!ret) return -1;

	    int startTriangle = (int)rtxTriangles.size();

	    for (const auto& shape : shapes) {
	        size_t index_offset = 0;

	        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
	            int fv = shape.mesh.num_face_vertices[f];

	            for (int i = 1; i + 1 < fv; i++) {
	                RtxTriangle tri{};

	                tinyobj::index_t idx0 = shape.mesh.indices[index_offset + 0];
	                tinyobj::index_t idx1 = shape.mesh.indices[index_offset + i];
	                tinyobj::index_t idx2 = shape.mesh.indices[index_offset + i + 1];

	                tri.v0 = glm::vec3(
	                    attrib.vertices[3 * idx0.vertex_index + 0],
	                    attrib.vertices[3 * idx0.vertex_index + 1],
	                    attrib.vertices[3 * idx0.vertex_index + 2]
	                );
	                tri.v1 = glm::vec3(
	                    attrib.vertices[3 * idx1.vertex_index + 0],
	                    attrib.vertices[3 * idx1.vertex_index + 1],
	                    attrib.vertices[3 * idx1.vertex_index + 2]
	                );
	                tri.v2 = glm::vec3(
	                    attrib.vertices[3 * idx2.vertex_index + 0],
	                    attrib.vertices[3 * idx2.vertex_index + 1],
	                    attrib.vertices[3 * idx2.vertex_index + 2]
	                );

	                tri.n0 = (idx0.normal_index >= 0) ? glm::vec3(
	                    attrib.normals[3 * idx0.normal_index + 0],
	                    attrib.normals[3 * idx0.normal_index + 1],
	                    attrib.normals[3 * idx0.normal_index + 2]
	                ) : glm::vec3(0.0f);

	                tri.n1 = (idx1.normal_index >= 0) ? glm::vec3(
	                    attrib.normals[3 * idx1.normal_index + 0],
	                    attrib.normals[3 * idx1.normal_index + 1],
	                    attrib.normals[3 * idx1.normal_index + 2]
	                ) : glm::vec3(0.0f);

	                tri.n2 = (idx2.normal_index >= 0) ? glm::vec3(
	                    attrib.normals[3 * idx2.normal_index + 0],
	                    attrib.normals[3 * idx2.normal_index + 1],
	                    attrib.normals[3 * idx2.normal_index + 2]
	                ) : glm::vec3(0.0f);

	                rtxTriangles.push_back(tri);
	            }

	            index_offset += fv;
	        }
	    }

	    int meshID = (int)rtxMeshes.size();

	    RtxMesh mesh{};
	    mesh.startTriangle = startTriangle;
	    mesh.numTriangles = (int)rtxTriangles.size() - startTriangle;

	    rtxMeshes.push_back(mesh);
	    meshLookup[name] = meshID;

	    trianglesChanged = true;
	    meshesChanged = true;

	    return meshID;
	}

	RtxMesh& AssetManager::GetRtxMesh(const std::string& name) {
	    int id = meshLookup.at(name);
	    return rtxMeshes[id];
	}

	void AssetManager::Clear() {
	    rtxMaterials.clear();
	    materialLookup.clear();
	    rtxTriangles.clear();
	    rtxMeshes.clear();
	    meshLookup.clear();

	    materialsChanged = true;
	    trianglesChanged = true;
	    meshesChanged = true;
	}
}