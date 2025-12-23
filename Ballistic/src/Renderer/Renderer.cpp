#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Project/ProjectManager.h"
#include "Project/Scene/Scene.h"
#include "Core/Config.h"

#include "Renderer/Backends/OpenGL/GLRenderDevice.h"

namespace Ballistic {

	Renderer::Renderer(std::shared_ptr<ProjectManager> projectManager) {
		m_projectManager = projectManager;
	}

	Renderer::~Renderer() {
		Shutdown();
	}

	void Renderer::RequestResize(glm::vec2 dim) {
		if (m_currentSize == dim) return;
		m_resizeSize = dim;
		m_pendingResize = true;
	}
	
	void Renderer::Init() {
		std::cout << "OpenGL Renderer Initialized" << std::endl;

		m_renderDevice = std::make_unique<GLRenderDevice>();
		m_renderDevice->Init();
	}
	
	void Renderer::Shutdown() {
		if (m_renderDevice) m_renderDevice->Shutdown();
	}

	void Renderer::Render() {
		if (m_pendingResize) {
			m_renderDevice->Resize((uint32_t)m_resizeSize.x, (uint32_t)m_resizeSize.y);
			m_currentSize = m_resizeSize;
			m_pendingResize = false;
		}

        // auto& scene = m_projectManager->GetSceneManager()->GetActiveScene();

		std::vector<RenderCommand> commands;

		auto& scene = m_projectManager->GetCurrentScene();
		for (auto entity : scene.GetAllEntitiesFlattened()) {

			RenderCommand cmd;
			cmd.modelMatrix = scene.ComputeWorldTransform(entity);

			commands.push_back(cmd);
		}

		m_renderDevice->Execute(commands);

	}
}