#include "OglRenderer.h"

namespace Ballistic {

	OglRenderer::OglRenderer(std::shared_ptr<IWindow> window) {
		m_Window = window;
	}

	void OglRenderer::setViewportSize(glm::vec2 dim) {
		// texture->destroy();
		// texture->create(dim.x, dim.y, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	}
	
	void OglRenderer::Init() {
		std::cout << "OpenGL Renderer Initialized" << std::endl;

		const char* computeShaderSrc = R"(
		#version 430 core
		layout (local_size_x = 16, local_size_y = 16) in;

		layout (rgba32f, binding = 0) uniform image2D imgOutput;

		void main() {
			ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
			vec2 uv = vec2(pixel) / vec2(imageSize(imgOutput));
			
			// simple gradient raytracer
			vec3 color = vec3(uv, 0.5);
			imageStore(imgOutput, pixel, vec4(color, 1.0));
		}
		)";

		shader = std::make_shared<gl::Shader>();
		shader->create();
		shader->attachShader(GL_COMPUTE_SHADER, computeShaderSrc);
		shader->link();

		texture = std::make_shared<gl::Texture2D>();
		texture->create(800, 600, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    	texture->setParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}
	
	void OglRenderer::Shutdown() {
	}

	void OglRenderer::Render() {
		shader->use();
        texture->bindImage(0, GL_WRITE_ONLY);
        shader->dispatchCompute((800 + 15)/16, (600 + 15)/16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}