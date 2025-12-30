#pragma once
#include "bepch.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Renderer/RenderDevice/IRenderDevice.h"
#include "Renderer/Backends/OpenGL/all.h"

namespace ballistic
{
    struct Vertex;
    struct DrawElementsIndirectCommand;

    struct RenderParameters {
        glm::mat4 camView;
        glm::mat4 camProj;
        alignas(16) glm::vec3 camPos;
    };

    class GLRenderDevice : public IRenderDevice {
    public:
        GLRenderDevice() = default;
        virtual ~GLRenderDevice() { Shutdown(); }

        bool Init() override;
        void Shutdown() override;

        void Clear(float r, float g, float b, float a) override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        void Execute(const std::vector<DrawElementsIndirectCommand>& commands) override;
		void BlitToScreen() override;

        void* GetNativeTextureHandle() override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_outputTexture)); }

        RenderParameters m_renderParams{};

    private:
        GLuint m_mainFramebuffer = 0;
        GLuint m_outputTexture = 0;
        GLuint m_depthRbo = 0;
        
        GLuint m_blitShader = 0;

        GLuint m_meshVAO = 0;
        GLuint m_meshVBO = 0;
        GLuint m_meshEBO = 0;
        GLuint m_indirectBuffer = 0;
        
        size_t m_vertexCapacityBytes = 0;
        size_t m_indexCapacityBytes  = 0;
        size_t m_indirectCapacityBytes = 0;

        GLuint m_renderParamsUBO = 0; // UBO binding 0
        
        GLuint tempShader = 0;

        glm::vec2 viewportSize;

        void InitMeshBuffers();
        void InitIndirectBuffer();
        void InitShaderBuffers();
        void InitFramebuffer(uint32_t w, uint32_t h);
        void InitShaders();
        
        void EnsureVertexBuffer(const Vertex* data, size_t count);
        void EnsureIndexBuffer(const uint32_t* data, size_t count);
        void EnsureIndirectBuffer(const DrawElementsIndirectCommand* data, size_t count);

        GLuint CreateShader(const char* vertexSrc, const char* fragmentSrc);
    };

} // namespace ballistic
