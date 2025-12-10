#pragma once

#include "lrpch.h"

struct alignas(16) RtxMaterial {
	glm::vec3 albedo{1.0f};
	float roughness = 0.0f;
	glm::vec3 emissionColor{0.0f};
	float emissionStrength = 0.0f;
};

struct alignas(16) RtxTriangle {
	glm::vec3 v0, v1, v2;
	glm::vec3 n0, n1, n2;
};

struct alignas(16) RtxMesh {
	int startTriangle;
	int numTriangles;
	float pad[2];

	// std::span<RtxTriangle> GetTriangles(std::vector<RtxTriangle>& allTriangles) const {
    //     return std::span(&allTriangles[startTriangle], numTriangles);
    // }
};

struct alignas(16) RtxInstance {
	glm::mat4 transform;
	int meshID;
	int materialID = -1;
	uint32_t entityID = -1;
	float pad[1];
};

struct alignas(16) RtxSphere {
	glm::vec3 center;
	float radius;
	int materialID = -1;
	uint32_t entityID = -1;
	float pad[2];
};