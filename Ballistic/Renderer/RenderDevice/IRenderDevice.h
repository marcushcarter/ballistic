#pragma once
#include "bepch.h"

namespace ballistic
{
    struct DrawElementsIndirectCommand;

    struct RenderParameters {
        glm::mat4 camView;
        glm::mat4 camProj;
        alignas(16) glm::vec3 camPos;
    };
    
    class IRenderDevice {
    public:
        virtual ~IRenderDevice() = default;

        virtual bool Init() = 0;
        virtual void Shutdown() = 0;

        virtual void Execute(const std::vector<DrawElementsIndirectCommand>& commands, const std::vector<glm::mat4>& instanceMatrices) = 0;
        
        virtual void Clear(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0) = 0;
        virtual void BlitToScreen() = 0;

        virtual void Resize(uint32_t newWidth = 0, uint32_t newHeight = 0) = 0;

        virtual void* GetNativeTextureHandle() = 0;
        
        RenderParameters m_renderParams{};
    };

} // namespace ballistic
