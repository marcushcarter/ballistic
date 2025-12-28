#include "Renderer/Renderer.h"
#include "Core/Window/Window.h"
#include "Renderer/RenderCommand.h"
#include "Core/LogManager/Log.h"

#include "Renderer/RenderDevice/GLRenderDevice.h"

namespace ballistic {

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
				
			LogDebug("OpenGL renderer Initialized");
		} else if (m_api == RendererAPI::Vulkan) {
			LogDebug("No Vulkan renderer yet");
			return false;
		} else {
			LogDebug("Unknown renderer chosen");
			return false;
		}

		return true;
	}
	
	void Renderer::Shutdown() {
		if (m_renderDevice) m_renderDevice->Shutdown();
	}

	void Renderer::OnUpdate(/* pass in current scene */) {
		if (m_pendingResize) {
			m_renderDevice->Resize((uint32_t)m_resizeSize.x, (uint32_t)m_resizeSize.y);
			m_currentSize = m_resizeSize;
			m_pendingResize = false;
		}

        // auto& scene = m_projectManager->GetSceneManager()->GetActiveScene();

		std::vector<RenderCommand> commands;

		// auto& scene = m_projectManager->GetCurrentScene();
		// for (auto entity : scene.GetAllEntitiesFlattened()) {

		// 	RenderCommand cmd;
		// 	cmd.modelMatrix = scene.ComputeWorldTransform(entity);

		// 	commands.push_back(cmd);
		// }

		m_renderDevice->Execute(commands);

	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_currentSize == dim) return;
		m_resizeSize = dim;
		m_pendingResize = true;
	}
}