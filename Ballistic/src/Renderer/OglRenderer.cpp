#include "OglRenderer.h"

namespace Ballistic {

	OglRenderer::OglRenderer(std::shared_ptr<IWindow> window) {
		m_Window = window;
	}

	void OglRenderer::requestResize(glm::vec2 dim) {
		m_ResizeSize = dim;
		m_PendingResize = true;
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
		if (m_PendingResize) {
			texture->destroy();
			texture->create(m_ResizeSize.x, m_ResizeSize.y, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			texture->setParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			m_PendingResize = true;
		}
		shader->use();
        texture->bindImage(0, GL_WRITE_ONLY);
        shader->dispatchCompute((texture->width() + 15)/16, (texture->height() + 15)/16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}