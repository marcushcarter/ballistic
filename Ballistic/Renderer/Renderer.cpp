#include "Renderer/Renderer.h"
#include "Core/Window/Window.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Scene/Components.h"
#include "Scene/EntityHandle.h"
#include "Scene/Scene.h"

#include "Root/LogManager/Log.h"

#include "Core/IApplication.h"
#include "Root/Root.h"

namespace ballistic
{
	void Renderer::ApplyWindowHints() {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	
	bool Renderer::Init(Window* window) {
		glfwMakeContextCurrent(window->GetNativeWindow());

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			LogError("Failed to initialize GLAD");
			window->Shutdown();
			return false;
		}
		
		InitShaderBuffers();
		InitFramebuffer(800, 600);
		InitShaders();
		
		LogInfo("Renderer: ", glGetString(GL_RENDERER));
		LogInfo("Vendor: ", glGetString(GL_VENDOR));
		LogInfo("Version: ", glGetString(GL_VERSION));
		LogInfo("GLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

		LogDebug("OpenGL renderer Initialized");
		return true;
	}
    
	void Renderer::InitShaderBuffers() {

		glGenBuffers(1, &m_instanceSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_instanceSSBO);
		m_instanceCapacityBytes = 1024 * sizeof(glm::mat4);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_instanceCapacityBytes, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_instanceSSBO);

		glGenBuffers(1, &m_renderParamsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_renderParamsUBO);
		m_renderParamsCapacityBytes = sizeof(RenderParameters);
		glBufferData(GL_UNIFORM_BUFFER, m_renderParamsCapacityBytes, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_renderParamsUBO);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
    void Renderer::InitFramebuffer(uint32_t w, uint32_t h) {
		if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);
		if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
		if (m_mainFramebuffer) glDeleteFramebuffers(1, &m_mainFramebuffer);

		glGenTextures(1, &m_outputTexture);
		glBindTexture(GL_TEXTURE_2D, m_outputTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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

		GLenum attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LogError("Framebuffer incomplete");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::InitShaders() {
		
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

		layout(std140, binding = 0) uniform ParametersUBO {
			mat4 camView;
			mat4 camProj;
			vec3 camPos;
			float _padding;
		};

		layout(std430, binding = 0) buffer InstanceBuffer {
			mat4 uModel[];
		};

		out vec3 FragPos;
		out vec3 Normal;
		out vec2 UV;

		void main() {
			mat4 model = uModel[gl_InstanceID];
			FragPos = vec3(model * vec4(aPos, 1.0));
			Normal = mat3(transpose(inverse(model))) * aNormal;
			gl_Position = camProj * camView * vec4(FragPos, 1.0);
			UV = aUV;
		}
		)";
		
		const char* sceneFrag = R"(
		#version 460 core

		layout(std140, binding = 0) uniform ParametersUBO {
			mat4 camView;
			mat4 camProj;
			vec3 camPos;
			float _padding;
		};

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

	void Renderer::EnsureRenderParamsUBO(const RenderParameters* data) {
		size_t dataBytes = sizeof(RenderParameters);
		glBindBuffer(GL_UNIFORM_BUFFER, m_renderParamsUBO);

		if (dataBytes > m_renderParamsCapacityBytes) {
			m_renderParamsCapacityBytes = std::max(dataBytes, m_renderParamsCapacityBytes * 2);
			glBufferData(GL_UNIFORM_BUFFER, m_renderParamsCapacityBytes, data, GL_DYNAMIC_DRAW);
			LogWarn("RenderParams UBO resized - new size: ", m_renderParamsCapacityBytes);
		} else {
			glBufferSubData(GL_UNIFORM_BUFFER, 0, dataBytes, data);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::EnsureInstanceSSBO(const glm::mat4* data, size_t count) {
		size_t requiredBytes = count * sizeof(glm::mat4);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_instanceSSBO);

		if (requiredBytes > m_instanceCapacityBytes) {
			m_instanceCapacityBytes = std::max(requiredBytes, m_instanceCapacityBytes * 2);
			glBufferData(GL_SHADER_STORAGE_BUFFER, m_instanceCapacityBytes, data, GL_DYNAMIC_DRAW);
			LogWarn("Instance SSBO resized - new size: ", m_instanceCapacityBytes);
		} else {
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, requiredBytes, data);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void Renderer::Render(Scene* scene) {
		if (m_pendingResize) {			
			glViewport(0, 0, m_resizeSize.x, m_resizeSize.y);
			InitFramebuffer(m_resizeSize.x, m_resizeSize.y);
			m_currentSize = m_resizeSize;
			m_pendingResize = false;
		}

		if (!scene) return;

		struct MeshInstance {
			const MeshMetadata* meta;
			std::vector<glm::mat4> models;
		};
		std::unordered_map<GUID, MeshInstance> meshInstances;

		for (auto entity : scene->GetAllEntitiesFlattened()) {
			EntityHandle e(entity, scene->GetRegistry());
			if (!e.has<MeshComponent>() || !e.has<TransformComponent>()) continue;

			GUID meshID = e.get<MeshComponent>().mesh;
			const MeshMetadata* meta = GetRoot()->GetMeshManager()->GetMeshMetadata(meshID);
			if (!meta) continue;
			
			auto& entry = meshInstances[meshID];
			entry.meta = meta;
			entry.models.push_back(e.get<TransformComponent>().TRS());
		}

		std::vector<DrawElementsIndirectCommand> commands;
		std::vector<glm::mat4> instanceMatrices;
		size_t baseInstance = 0;

		for (auto& [meshID, entry] : meshInstances) {
			DrawElementsIndirectCommand cmd{};
			cmd.count = entry.meta->indexCount;
			cmd.instanceCount = (uint32_t)entry.models.size();
			cmd.firstIndex = entry.meta->indexOffset;
			cmd.baseVertex = entry.meta->vertexOffset;
			cmd.baseInstance = (uint32_t)baseInstance;

			commands.push_back(cmd);

			instanceMatrices.insert(instanceMatrices.end(), entry.models.begin(), entry.models.end());
			baseInstance += entry.models.size();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		EnsureRenderParamsUBO(&m_renderParams);

		auto meshManager = GetRoot()->GetMeshManager();
		glBindVertexArray(meshManager->GetVAO());
		// meshManager->EnsureBuffers();

		EnsureInstanceSSBO(instanceMatrices.data(), instanceMatrices.size());

		glUseProgram(tempShader);
		glBindVertexArray(meshManager->GetVAO());

		for (auto& cmd : commands) {
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				cmd.count,
				GL_UNSIGNED_INT,
				(void*)(cmd.firstIndex * sizeof(uint32_t)),
				cmd.instanceCount,
				cmd.baseVertex
			);
		}

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	void Renderer::SubmitCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& pos) {
		m_renderParams.camView = view;
		m_renderParams.camProj = proj;
		m_renderParams.camPos  = pos;
	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_currentSize == dim) return;
		m_resizeSize = dim;
		m_pendingResize = true;
	}
	
	GLuint Renderer::CreateShader(const char* vertexSrc, const char* fragmentSrc) {
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
	
	void Renderer::Shutdown() {
		glDeleteFramebuffers(1, &m_mainFramebuffer);
		glDeleteRenderbuffers(1, &m_depthRbo);
		glDeleteTextures(1, &m_outputTexture);
		
		glDeleteProgram(m_blitShader);
		glDeleteProgram(tempShader);
	}

} // namespace ballistic
