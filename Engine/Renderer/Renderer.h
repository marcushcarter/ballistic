#pragma once
#include "bepch.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/RendererTypes.h"

namespace Ballistic {

	class ProjectManager;

	class Renderer {
	public:
		Renderer(std::shared_ptr<ProjectManager> projectManager);
		~Renderer();

		void Init();
		void Shutdown();
		void Render();

		void RequestResize(glm::vec2 dim);

		RenderDevice* GetDevice() const { return m_renderDevice.get(); }
		glm::vec2 GetSize() const { return m_currentSize; }

	private:
		std::shared_ptr<ProjectManager> m_projectManager;
		std::unique_ptr<RenderDevice> m_renderDevice;

		glm::vec2 m_currentSize{};
		glm::vec2 m_resizeSize{};
		bool m_pendingResize = true;

	};
}