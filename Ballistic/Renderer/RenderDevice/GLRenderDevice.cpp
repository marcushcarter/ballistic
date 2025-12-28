#include "Renderer/RenderDevice/GLRenderDevice.h"
#include "Core/IApplication.h"
#include "Core/LogManager/Log.h"

namespace ballistic {

    bool GLRenderDevice::Init() {

		shader = std::make_shared<gl::Shader>();
		shader->create();
		shader->attachShader(GL_COMPUTE_SHADER, IApplication::GetResDirectory() / "Shaders/pathTracing.comp");
		shader->link();

		m_outputTexture = std::make_shared<gl::Texture2D>();
		m_outputTexture->create(800, 600, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    	m_outputTexture->setParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		m_mainFramebuffer = std::make_shared<gl::Framebuffer>();
		m_mainFramebuffer->create();
		m_mainFramebuffer->attachColor(0, *m_outputTexture);

		gl::Renderbuffer depthBuffer;
		depthBuffer.create();
		depthBuffer.storage(GL_DEPTH_COMPONENT24, 800, 600);
		m_mainFramebuffer->attachDepth(depthBuffer);

		if (!m_mainFramebuffer->complete()) {
			LogError("Framebuffer incomplete");
			return false;
		}

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
		
		m_blitShader = std::make_shared<gl::Shader>();
		m_blitShader->create();
		m_blitShader->attachShader(GL_VERTEX_SHADER, vertexShaderSrc);
		m_blitShader->attachShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
		m_blitShader->link();

		m_blitVAO = std::make_shared<gl::VertexArray>();
		m_blitVAO->create();

		return true;
    }

    void GLRenderDevice::Shutdown() {	
		m_outputTexture->destroy();
		m_mainFramebuffer->destroy();
	  
		m_blitShader->destroy();
		m_blitVAO->destroy();
    }

    void GLRenderDevice::Execute(const std::vector<RenderCommand>& commands) {
		if (false) {
			shader->use();
        	m_outputTexture->bindImage(0, GL_WRITE_ONLY);
        	shader->dispatchCompute((m_outputTexture->width() + 15)/16, (m_outputTexture->height() + 15)/16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		} else {
			m_mainFramebuffer->bind();
			gl::Viewport(0, 0, m_outputTexture->width(), m_outputTexture->height());
        	gl::ClearColor(rgb.x, rgb.y, rgb.z, 1.0f);
			gl::Clear();
			m_mainFramebuffer->unbind();
		}

    }
    
	void GLRenderDevice::Clear(float r, float g, float b, float a) {
		gl::ClearColor(r, g, b, a);
		gl::Clear();
	}
    
	void GLRenderDevice::BlitToScreen() {
		gl::State::bindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		gl::Viewport(0, 0, m_outputTexture->width(), m_outputTexture->height());

		m_blitShader->use();
		m_outputTexture->bind(0);
		m_blitShader->setUniform<int>("screenTexture", 0);

		m_blitVAO->bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		m_blitVAO->unbind();
	}

    void GLRenderDevice::Resize(uint32_t w, uint32_t h) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0f, 1.0f);
		rgb = glm::vec3(dis(gen), dis(gen), dis(gen));

        m_outputTexture->resize(w, h);

        m_mainFramebuffer->destroy();
        m_mainFramebuffer->create();
        m_mainFramebuffer->attachColor(0, *m_outputTexture);

        gl::Renderbuffer depthBuffer;
        depthBuffer.create();
        depthBuffer.storage(GL_DEPTH_COMPONENT24, w, h);
        m_mainFramebuffer->attachDepth(depthBuffer);
    }

    void* GLRenderDevice::GetNativeTextureHandle() {
        return (void*)(uintptr_t)m_outputTexture->get();
    }
        
}