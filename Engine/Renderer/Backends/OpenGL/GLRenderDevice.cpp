#include "GLRenderDevice.h"
#include "Core/Config.h"
#include <random>

namespace Ballistic {

    void GLRenderDevice::Init() {

		shader = std::make_shared<gl::Shader>();
		shader->create();
		shader->attachShader(GL_COMPUTE_SHADER, Config::RESOURCES_PATH / "Shaders/pathTracing.comp");
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

		const char* vertexShaderSrc = R"(
		#version 460 core
		out vec2 TexCoords;
		void main() {
			const vec2 verts[3] = vec2[3](
				vec2(-1.0, -1.0),
				vec2( 3.0, -1.0),
				vec2(-1.0,  3.0)
			);
			gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
			TexCoords = (gl_Position.xy + 1.0) * 0.5;
		}
		)";
		
		const char* fragmentShaderSrc = R"(
		#version 460 core
		in vec2 TexCoords;
		out vec4 FragColor;
		uniform sampler2D screenTexture;
		void main() {
			FragColor = texture(screenTexture, TexCoords);
		}
		)";
		
		blitShader = std::make_shared<gl::Shader>();
		blitShader->create();
		blitShader->attachShader(GL_VERTEX_SHADER, vertexShaderSrc);
		blitShader->attachShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
		blitShader->link();

		blitVAO = std::make_shared<gl::VertexArray>();
		blitVAO->create();

    }

    void GLRenderDevice::Shutdown() {

    }

    void GLRenderDevice::Execute(const std::vector<RenderCommand>& commands) {
		if (false) {
			shader->use();
        	texture->bindImage(0, GL_WRITE_ONLY);
        	shader->dispatchCompute((texture->width() + 15)/16, (texture->height() + 15)/16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		} else {
			framebuffer->bind();
			gl::Viewport(0, 0, texture->width(), texture->height());
        	gl::ClearColor(rgb.x, rgb.y, rgb.z, 1.0f);
			gl::Clear();
			framebuffer->unbind();
		}

    }
    
	void GLRenderDevice::Clear(float r, float g, float b, float a) {
		gl::ClearColor(r, g, b, a);
		gl::Clear();
	}
    
	void GLRenderDevice::BlitToScreen() {
		gl::State::bindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		gl::Viewport(0, 0, texture->width(), texture->height());

		blitShader->use();
		texture->bind(0);
		blitShader->setUniform<int>("screenTexture", 0);

		blitVAO->bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		blitVAO->unbind();
	}

    void GLRenderDevice::Resize(uint32_t w, uint32_t h) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0f, 1.0f);
		rgb = glm::vec3(dis(gen), dis(gen), dis(gen));

        texture->resize(w, h);

        framebuffer->destroy();
        framebuffer->create();
        framebuffer->attachColor(0, *texture);

        gl::Renderbuffer depthBuffer;
        depthBuffer.create();
        depthBuffer.storage(GL_DEPTH_COMPONENT24, w, h);
        framebuffer->attachDepth(depthBuffer);
    }

    void* GLRenderDevice::GetNativeTextureHandle() {
        return (void*)(uintptr_t)texture->get();
    }
        
}