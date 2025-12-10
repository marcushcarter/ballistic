#pragma once

#include "lrpch.h"
#include "../Platform/OpenGL/Image2D.h"
#include "../Platform/OpenGL/Shader.h"

namespace Ballistic {

	class Renderer {
	public:
		void Init();

		// void setViewportSize(glm::vec2 dim);

		void InitComputeRtxStage();
		Image2D* RenderComputeRtxStage();
		// RTXUniformParameters rtxUniformParameters;
		// RTXComputeStatistics rtxComputeStats;

		Image2D* computeRtxTexture;
		Shader* computeRtxShader;

	};

}