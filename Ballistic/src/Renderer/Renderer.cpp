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
    
	    computeRtxShader = new Shader;
	    computeRtxShader->attachSource(GL_COMPUTE_SHADER, source);
	    computeRtxShader->link();

	    // rtxComputeStats.timeQuery = new Query();

	    // ConfigureRtxParametersUBO();
	    // ConfigureRtxTrianglesSSBO();
	    // ConfigureRtxMeshesSSBO();
	    // ConfigureRtxMaterialsSSBO();
	    // ConfigureRtxInstancesSSBO();
	    // ConfigureRtxSpheresSSBO();

	    computeRtxTexture = new Image2D(1200, 800);
	}

	Image2D* Renderer::RenderComputeRtxStage() {
		computeRtxShader->bind();

		// rtxInstances.clear();
		// rtxSpheres.clear();

		// UpdateRtxParametersUBO();
		// UpdateRtxTrianglesSSBO();
		// UpdateRtxMeshesSSBO();
		// UpdateRtxMaterialsSSBO();
		// UpdateRtxInstancesSSBO();
		// UpdateRtxSpheresSSBO();

		// rtxComputeStats.primitivesDrawn = rtxComputeStats.spheresDrawn + rtxComputeStats.trianglesDrawn;

		// rtxUniformParameters.numInstances = (int)rtxInstances.size();
		// rtxUniformParameters.numSpheres = (int)rtxSpheres.size();

	    // rtxComputeStats.timeQuery->begin();

		computeRtxTexture->bind(0);
		int groupX = (computeRtxTexture->m_Width + 15) / 16;
	    int groupY = (computeRtxTexture->m_Height + 15) / 16;
		computeRtxShader->dispatchCompute(groupX, groupY);
		computeRtxTexture->unbind(0);

	    // rtxComputeStats.raysCast = computeRtxTexture->m_Width * computeRtxTexture->m_Height * 1 * 1; // rays per pixel / bounce count

	    // rtxComputeStats.timeQuery->end();
	    // rtxComputeStats.gpuTime = rtxComputeStats.timeQuery->result() / 1000000.0f;

		return computeRtxTexture;
	}
	
}