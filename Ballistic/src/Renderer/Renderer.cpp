#include "Renderer/Renderer.h"
#include "Core/Config.h"

namespace Ballistic {

	Renderer::Renderer() {
	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_CurrentSize == dim) return;
		m_ResizeSize = dim;
		m_PendingResize = true;
	}
	
	void Renderer::Init() {
		std::cout << "OpenGL Renderer Initialized" << std::endl;

		m_PendingResize = true;
		m_ResizeSize = glm::vec2(800, 600);

		shader = std::make_shared<gl::Shader>();
		shader->create();
		shader->attachShader(GL_COMPUTE_SHADER, Config::BALLISTIC_RES_PATH / "Shaders/pathTracing.comp");
		shader->link();

		texture = std::make_shared<gl::Texture2D>();
		texture->create(800, 600, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    	texture->setParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		framebuffer = std::make_shared<gl::Framebuffer>();
		framebuffer->create();
		framebuffer->attachColor(0, *texture);

		gl::Renderbuffer depthBuffer;
		depthBuffer.create();
		depthBuffer.storage(GL_DEPTH_COMPONENT24, 800, 600);
		framebuffer->attachDepth(depthBuffer);

		if (!framebuffer->complete())
        	throw std::runtime_error("Framebuffer incomplete!");
	}
	
	void Renderer::Shutdown() {
	}

	void Renderer::Render() {
		if (m_PendingResize) {
			int w = (int)m_ResizeSize.x;
        	int h = (int)m_ResizeSize.y;

			texture->resize(w, h);

			framebuffer->destroy();
			framebuffer->create();
			framebuffer->attachColor(0, *texture);

			gl::Renderbuffer depthBuffer;
			depthBuffer.create();
			depthBuffer.storage(GL_DEPTH_COMPONENT24, w, h);
			framebuffer->attachDepth(depthBuffer);

			m_PendingResize = false;
		}

		if (true) {
			shader->use();
        	texture->bindImage(0, GL_WRITE_ONLY);
        	shader->dispatchCompute((texture->width() + 15)/16, (texture->height() + 15)/16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		} else {
			framebuffer->bind();
			gl::Viewport(0, 0, texture->width(), texture->height());
			gl::ClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			gl::Clear();
			framebuffer->unbind();
		}

	}
}