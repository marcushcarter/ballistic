#include "Renderer.h"

namespace Ballistic {

	void Renderer::Init() {
		InitComputeRtxStage();
	}

	void Renderer::InitComputeRtxStage() {

		const char* source = R"(

		#version 460

		layout(local_size_x = 16, local_size_y = 16) in;

		layout(rgba32f, binding = 0) uniform image2D computeOutput;

		void main() {
		    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
		    ivec2 texSize = imageSize(computeOutput);
		    vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(texSize);
		    imageStore(computeOutput, pixel, vec4(uv, 1.0, 1.0));
		}

		)";
    
	    computeRtxShader = std::make_shared<Shader>();;
	    computeRtxShader->attachSource(GL_COMPUTE_SHADER, source);
	    computeRtxShader->link();

	    // rtxComputeStats.timeQuery = new Query();

	    // ConfigureRtxParametersUBO();
	    // ConfigureRtxTrianglesSSBO();
	    // ConfigureRtxMeshesSSBO();
	    // ConfigureRtxMaterialsSSBO();
	    // ConfigureRtxInstancesSSBO();
	    // ConfigureRtxSpheresSSBO();

	    computeRtxTexture = std::make_shared<Image2D>(1200, 800);
	}

	std::shared_ptr<Image2D> Renderer::RenderComputeRtxStage() {
		computeRtxShader->bind();
		computeRtxTexture->bind(0);

		// rtxInstances.clear();
		// rtxSpheres.clear();

		// UpdateRtxParametersUBO();
		// UpdateRtxTrianglesSSBO();
		// UpdateRtxMeshesSSBO();
		// UpdateRtxMaterialsSSBO();
		// UpdateRtxInstancesSSBO();
		// UpdateRtxSpheresSSBO();

		int groupX = (computeRtxTexture->m_Width + 15) / 16;
	    int groupY = (computeRtxTexture->m_Height + 15) / 16;
		computeRtxShader->dispatchCompute(groupX, groupY);

		computeRtxTexture->unbind(0);

		return computeRtxTexture;
	}
	
}