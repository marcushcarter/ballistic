#pragma once

#include "lrpch.h"
#include "RtxPrimitives.h"

namespace Ballistic {

#pragma once

	class AssetManager {
	public:
		std::vector<RtxMaterial> rtxMaterials;
		std::vector<RtxMesh> rtxMeshes;
		std::vector<RtxTriangle> rtxTriangles;

		std::unordered_map<std::string, int> materialLookup;
		std::unordered_map<std::string, int> meshLookup;

		bool materialsChanged = false;
		bool trianglesChanged = false;
		bool meshesChanged = false;

		int AddRtxMaterial(const std::string& name, const RtxMaterial& mat);
		RtxMaterial& GetRtxMaterial(const std::string& name);

		int AddRtxMesh(const std::string& name, const std::string& path);
		RtxMesh& GetRtxMesh(const std::string& name);

		void Clear();

	};

}