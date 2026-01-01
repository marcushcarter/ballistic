#include "Renderer/Renderer.h"
#include "Core/Window/Window.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Scene/Components.h"
#include "Scene/EntityHandle.h"
#include "Scene/Scene.h"

#include "Root/LogManager/Log.h"

#include "Renderer/RenderDevice/GLRenderDevice.h"

namespace ballistic
{
	void Renderer::ApplyWindowHints() {
		if (m_api == RendererAPI::OpenGL) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		} else if (m_api == RendererAPI::Vulkan) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
	}
	
	bool Renderer::Init(Window* window) {
		if (m_api == RendererAPI::OpenGL) {		
			glfwMakeContextCurrent(window->GetNativeWindow());

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
				LogError("Failed to initialize GLAD");
				window->Shutdown();
				return false;
			}

			m_renderDevice = std::make_unique<GLRenderDevice>();
			if (!m_renderDevice->Init())
				return false;
				
		} else if (m_api == RendererAPI::Vulkan) {
			LogDebug("No Vulkan renderer yet");
			return false;
		} else {
			LogDebug("Unknown renderer chosen");
			return false;
		}

		LogDebug("Renderer Initialized");
		return true;
	}
	
	void Renderer::Shutdown() {
		if (m_renderDevice) m_renderDevice->Shutdown();
	}

	void Renderer::OnUpdate(Scene* scene) {
		if (m_pendingResize) {
			m_renderDevice->Resize((uint32_t)m_resizeSize.x, (uint32_t)m_resizeSize.y);
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
			entry.models.push_back(scene->ComputeWorldTransform(e.handle()));
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

		m_renderDevice->Execute(commands, instanceMatrices);
	}
	
	void Renderer::SubmitCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& pos) {
		m_renderDevice->m_renderParams.camView = view;
		m_renderDevice->m_renderParams.camProj = proj;
		m_renderDevice->m_renderParams.camPos  = pos;
	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_currentSize == dim) return;
		m_resizeSize = dim;
		m_pendingResize = true;
	}

} // namespace ballistic
