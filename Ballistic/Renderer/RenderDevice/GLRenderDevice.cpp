#include "Renderer/RenderDevice/GLRenderDevice.h"
#include "Core/IApplication.h"
#include "Root/Root.h"
#include "Root/LogManager/Log.h"
#include "Renderer/Backends/OpenGL/ShaderGLM.h"

namespace ballistic
{
    bool GLRenderDevice::Init() {
		InitMeshBuffers();
		InitIndirectBuffer();
		InitFramebuffer(800, 600);
		InitShaders();

		LogInfo(
			"OpenGL Info: ", 
			"Renderer: ", glGetString(GL_RENDERER), ", ",
			"Vendor: ", glGetString(GL_VENDOR), ", ",
			"Version: ", glGetString(GL_VERSION), ", ",
			"GLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION)
		);

		LogDebug("OpenGL render device Initialized");
		return true;
    }

	void GLRenderDevice::InitMeshBuffers() {

		auto manager = GetRoot()->GetMeshManager();
		auto vertices = manager->GetVertexBuffer();
		auto indices = manager->GetIndexBuffer();

		glGenVertexArrays(1, &m_meshVAO);
		glBindVertexArray(m_meshVAO);

		glGenBuffers(1, &m_meshVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshVBO);
		m_vertexCapacityBytes = 1000 * sizeof(Vertex);
		glBufferData(GL_ARRAY_BUFFER, m_vertexCapacityBytes, nullptr, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &m_meshEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshEBO);
		m_indexCapacityBytes = 3000 * sizeof(uint32_t);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCapacityBytes, nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void GLRenderDevice::InitIndirectBuffer() {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
		m_indirectCapacityBytes = 1024 * sizeof(DrawElementsIndirectCommand);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, m_indirectCapacityBytes, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}
    
	void GLRenderDevice::InitShaderBuffers() {
		glGenBuffers(1, &m_renderParamsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_renderParamsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(RenderParameters), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_renderParamsUBO);
	}
	
    void GLRenderDevice::InitFramebuffer(uint32_t w, uint32_t h) {
		if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);
		if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
		if (m_mainFramebuffer) glDeleteFramebuffers(1, &m_mainFramebuffer);

		glGenTextures(1, &m_outputTexture);
		glBindTexture(GL_TEXTURE_2D, m_outputTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffers(1, &m_depthRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glGenFramebuffers(1, &m_mainFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTexture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LogError("Framebuffer incomplete");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLRenderDevice::InitShaders() {
		
		const char* blitVert = R"(
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
		
		const char* blitFrag = R"(
		#version 460 core
		in vec2 TexCoords;
		out vec4 FragColor;
		uniform sampler2D screenTexture;
		void main() {
			FragColor = texture(screenTexture, TexCoords);
			// FragColor = vec4(TexCoords, 0.0, 1.0);
		}
		)";

		m_blitShader = CreateShader(blitVert, blitFrag);

		const char* sceneVert = R"(
		#version 460 core

		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec3 aNormal;
		layout(location = 2) in vec2 aUV;

		uniform mat4 uModel;
		uniform mat4 uView;
		uniform mat4 uProj;

		out vec3 FragPos;
		out vec3 Normal;
		out vec2 UV;

		void main() {
			FragPos = vec3(uModel * vec4(aPos, 1.0));
			Normal = mat3(transpose(inverse(uModel))) * aNormal;
			UV = aUV;
			gl_Position = uProj * uView * vec4(FragPos, 1.0);
		}
		)";
		
		const char* sceneFrag = R"(
		#version 460 core

		in vec3 FragPos;
		in vec3 Normal;
		in vec2 UV;

		out vec4 FragColor;

		void main() {
			FragColor = vec4(UV, 0.0, 1.0);
		}
		)";

		tempShader = CreateShader(sceneVert, sceneFrag);
	}

	// void GLRenderDevice::CreateMeshBuffers() {
	// }

	void GLRenderDevice::EnsureVertexBuffer(const Vertex* data, size_t count) {
		size_t requiredBytes = count * sizeof(Vertex);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshVBO);

		if (requiredBytes > m_vertexCapacityBytes) {
			m_vertexCapacityBytes = std::max(requiredBytes, m_vertexCapacityBytes * 2);
			glBufferData(GL_ARRAY_BUFFER, m_vertexCapacityBytes, data, GL_DYNAMIC_DRAW);
			LogWarn("Vertex buffer resized - new size: ", m_vertexCapacityBytes);
		} else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, requiredBytes, data);
		}
    }

    void GLRenderDevice::EnsureIndexBuffer(const uint32_t* data, size_t count) {
		size_t requiredBytes = count * sizeof(uint32_t);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshEBO);

		if (requiredBytes > m_indexCapacityBytes) {
			m_indexCapacityBytes = std::max(requiredBytes, m_indexCapacityBytes * 2);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCapacityBytes, data, GL_DYNAMIC_DRAW);
			LogWarn("Index buffer resized - new size: ", m_indexCapacityBytes);
		} else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, requiredBytes, data);
		}
    }

    void GLRenderDevice::EnsureIndirectBuffer(const DrawElementsIndirectCommand* data, size_t count) {
		size_t requiredBytes = count * sizeof(uint32_t);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);

		if (requiredBytes > m_indirectCapacityBytes) {
			m_indirectCapacityBytes = std::max(requiredBytes, m_indirectCapacityBytes * 2);
			glBufferData(GL_DRAW_INDIRECT_BUFFER, m_indirectCapacityBytes, data, GL_DYNAMIC_DRAW);
			LogWarn("Indirect buffer resized - new size: ", m_indirectCapacityBytes);
		} else {
			glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, requiredBytes, data);
		}
    }

    void GLRenderDevice::Execute(const std::vector<DrawElementsIndirectCommand>& commands) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
		Clear(1.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		auto meshManager = GetRoot()->GetMeshManager();
		auto& vertices = meshManager->GetVertexBuffer();
		auto& indices = meshManager->GetIndexBuffer();

		if (!meshManager->GetDirtyMetadata().empty()) {
			glBindVertexArray(m_meshVAO);

			EnsureVertexBuffer(vertices.data(), vertices.size());
			EnsureIndexBuffer(indices.data(), indices.size());

			glBindVertexArray(0);
			LogWarn("Updated VAO");
			meshManager->ClearDirty();
		}

		EnsureIndirectBuffer(commands.data(), commands.size());

		glUseProgram(tempShader);

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)(viewportSize.x/viewportSize.y), 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(tempShader, "uModel"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(tempShader, "uView"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(tempShader, "uProj"), 1, GL_FALSE, &proj[0][0]);

		// glBindVertexArray(m_meshVAO);
		// glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
		// glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)commands.size(), 0);
		// glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		// glBindVertexArray(0);

		glBindVertexArray(m_meshVAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
	void GLRenderDevice::BlitToScreen() {
		glDisable(GL_DEPTH_TEST);
		glUseProgram(m_blitShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_outputTexture);
		glUniform1i(glGetUniformLocation(m_blitShader, "screenTexture"), 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
    
	void GLRenderDevice::Clear(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

    void GLRenderDevice::Resize(uint32_t newWidth, uint32_t newHeight) {
		glViewport(0, 0, newWidth, newHeight);
		viewportSize = glm::vec2(newWidth, newHeight);
		InitFramebuffer(newWidth, newHeight);
    }

	GLuint GLRenderDevice::CreateShader(const char* vertexSrc, const char* fragmentSrc) {
		auto compileShader = [](GLenum type, const char* src) -> GLuint {
			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &src, nullptr);
			glCompileShader(shader);

			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				char infoLog[1024];
				glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
				const char* typeName = (type == GL_VERTEX_SHADER) ? "VERTEX" : (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "UNKNOWN";
				LogError(typeName, " SHADER COMPILATION FAILED:\n", infoLog);
				glDeleteShader(shader);
				return 0;
			}

			return shader;
		};

		GLuint vert = compileShader(GL_VERTEX_SHADER, vertexSrc);
		if (!vert) return 0;

		GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
		if (!frag) {
			glDeleteShader(vert);
			return 0;
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, vert);
		glAttachShader(program, frag);
		glLinkProgram(program);

		GLint linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked) {
			char infoLog[1024];
			glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
			LogWarn("SHADER PROGRAM LINK FAILED:\n", infoLog);

			glDeleteProgram(program);
			glDeleteShader(vert);
			glDeleteShader(frag);
			return 0;
		}

		glDetachShader(program, vert);
		glDetachShader(program, frag);
		glDeleteShader(vert);
		glDeleteShader(frag);

		return program;
	}

    void GLRenderDevice::Shutdown() {
		glDeleteFramebuffers(1, &m_mainFramebuffer);
		glDeleteRenderbuffers(1, &m_depthRbo);
		glDeleteTextures(1, &m_outputTexture);
		
		glDeleteProgram(m_blitShader);
		glDeleteProgram(tempShader);

		glDeleteVertexArrays(1, &m_meshVAO);
		glDeleteBuffers(1, &m_meshVBO);
		glDeleteBuffers(1, &m_meshEBO);
		glDeleteBuffers(1, &m_indirectBuffer);
    }
        
} // namespace ballistic
