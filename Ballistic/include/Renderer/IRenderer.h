#pragma once
#include "bepch.h"

namespace Ballistic {

	class IRenderer {
	public:
		IRenderer();

		void Init();
		void Shutdown();
		void Render();

		void RequestResize(glm::vec2 dim);

		std::shared_ptr<gl::Texture2D> getTexture() const { return texture; }
		std::shared_ptr<gl::Framebuffer> getFramebuffer() const { return framebuffer; }

	private:
		std::shared_ptr<gl::Shader> shader;
		std::shared_ptr<gl::Texture2D> texture;
		std::shared_ptr<gl::Framebuffer> framebuffer;

		glm::vec2 m_CurrentSize;
		glm::vec2 m_ResizeSize;
		bool m_PendingResize;

	};
}