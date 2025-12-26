#pragma once
#include "bepch.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/RendererTypes.h"
#include "all.h"

namespace Ballistic {

    class GLRenderDevice : public RenderDevice {
    public:
        GLRenderDevice() = default;
        virtual ~GLRenderDevice() { Shutdown(); }

        void Init() override;
        void Shutdown() override;

        void Execute(const std::vector<RenderCommand>& commands) override;
		
        void Clear(float r, float g, float b, float a) override;
		void BlitToScreen() override;

        void Resize(uint32_t w, uint32_t h) override;

        void* GetNativeTextureHandle() override;

    private:
		std::shared_ptr<gl::Shader> shader;
		std::shared_ptr<gl::Texture2D> texture;
		std::shared_ptr<gl::Framebuffer> framebuffer;
	  
		std::shared_ptr<gl::Shader> blitShader;
		std::shared_ptr<gl::VertexArray> blitVAO;

      	glm::vec3 rgb;
        
    };
}