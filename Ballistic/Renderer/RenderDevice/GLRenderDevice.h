#pragma once
#include "bepch.h"
#include "Renderer/DrawElementsIndirectCommand.h"
#include "Renderer/RenderDevice/IRenderDevice.h"
#include "glad/glad.h"

namespace ballistic
{
    struct Vertex;
    struct DrawElementsIndirectCommand;

    class GLRenderDevice : public IRenderDevice {
    public:
        GLRenderDevice() = default;
        virtual ~GLRenderDevice() { Shutdown(); }

        bool Init() override;
        void Shutdown() override;

        void Clear(float r, float g, float b, float a) override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        void Execute(const std::vector<DrawElementsIndirectCommand>& commands, const std::vector<glm::mat4>& instanceMatrices) override;
		void BlitToScreen() override;

        void* GetNativeTextureHandle() override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_outputTexture)); }

    private:
        GLuint m_mainFramebuffer = 0;
        GLuint m_outputTexture = 0;
        GLuint m_depthRbo = 0;
        GLuint m_blitShader = 0;

        GLuint m_vertexArray = 0;

        GLuint m_vertexBuffer = 0;
        GLuint m_vertexSSBO = 0; // SSBO binding 1
        size_t m_vertexCapacityBytes = 0;
        
        GLuint m_indexBuffer = 0;
        GLuint m_indexSSBO = 0; // SSBO binding 2
        size_t m_indexCapacityBytes  = 0;

        GLuint m_instanceSSBO = 0; // SSBO binding 0
        size_t m_instanceCapacityBytes = 0;
        
        GLuint m_renderParamsUBO = 0; // UBO binding 0
        size_t m_renderParamsCapacityBytes = 0;

        void InitMeshBuffers();
        void InitShaderBuffers();
        void InitFramebuffer(uint32_t w, uint32_t h);
        void InitShaders();
        
        void EnsureVertexBuffer(const Vertex* data, size_t count);
        void EnsureIndexBuffer(const uint32_t* data, size_t count);
        void EnsureRenderParamsUBO(const RenderParameters* data);
        void EnsureInstanceSSBO(const glm::mat4* data, size_t count);

        GLuint CreateShader(const char* vertexSrc, const char* fragmentSrc);

        glm::vec2 viewportSize;

        GLuint tempShader = 0;
    };

} // namespace ballistic
