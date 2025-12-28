#pragma once
#include "bepch.h"
#include "Renderer/RenderDevice/IRenderDevice.h"

namespace ballistic {

	class Window;
	
	enum class RendererAPI { OpenGL, Vulkan };

	class Renderer {
	public:
		Renderer(RendererAPI api = RendererAPI::OpenGL) { m_api = api; }
		~Renderer() { Shutdown(); }

		void ApplyWindowHints();

		bool Init(Window* window);
		void Shutdown();
		void OnUpdate(/* pass in current scene */);

		void RequestResize(glm::vec2 dim);
		glm::vec2 GetSize() const { return m_currentSize; }
		
		IRenderDevice* GetDevice() const { return m_renderDevice.get(); }
		RendererAPI GetAPI() const { return m_api; }

	private:
		RendererAPI m_api;
		glm::vec2 m_currentSize{};
		glm::vec2 m_resizeSize{};
		bool m_pendingResize = true;

		std::unique_ptr<IRenderDevice> m_renderDevice;
	};
}