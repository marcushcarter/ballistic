#include "Renderer/Renderer.h"
#include "Core/Window/Window.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Scene/Components/Components.h"
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

		std::vector<DrawElementsIndirectCommand> commands;

		if (scene) {
			for (auto entity : scene->GetAllEntitiesFlattened()) {
				EntityHandle e(entity, scene->GetRegistry());

				if (e.has<MeshComponent>()) {
					const MeshMetadata* meta = GetRoot()->GetMeshManager()->GetMeshMetadata(e.get<MeshComponent>().mesh);
					if (!meta) continue;

					DrawElementsIndirectCommand cmd{};
					cmd.count = meta->indexCount;
					cmd.instanceCount = 1;
					cmd.firstIndex = meta->indexOffset;
					cmd.baseVertex = meta->vertexOffset;
					cmd.baseInstance = 0;

					commands.push_back(cmd);
				}
			}
		}

		m_renderDevice->Execute(commands);
	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_currentSize == dim) return;
		m_resizeSize = dim;
		m_pendingResize = true;
	}

} // namespace ballistic
