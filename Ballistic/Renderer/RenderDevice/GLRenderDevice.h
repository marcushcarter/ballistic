#pragma once
#include "bepch.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Renderer/RenderDevice/IRenderDevice.h"
#include "Renderer/Backends/OpenGL/all.h"

namespace ballistic
{
    class GLRenderDevice : public IRenderDevice {
    public:
        GLRenderDevice() = default;
        virtual ~GLRenderDevice() { Shutdown(); }

        bool Init() override;
        void Shutdown() override;

        void Execute(const std::vector<DrawElementsIndirectCommand>& commands) override;
		
        void Clear(float r, float g, float b, float a) override;
		void BlitToScreen() override;

        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        void* GetNativeTextureHandle() override;

    private:
        GLuint m_mainFramebuffer = 0;
        GLuint m_outputTexture = 0;
        GLuint m_depthRbo = 0;
        GLuint m_blitShader = 0;

        GLuint m_meshVAO = 0;
        GLuint m_meshVBO = 0;
        GLuint m_meshEBO = 0;
        GLuint m_indirectBuffer = 0;
        
        GLuint tempShader = 0;

        glm::vec2 viewportSize;

        void CreateFramebuffer(uint32_t w, uint32_t h);
    };

} // namespace ballistic
