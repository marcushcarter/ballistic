#pragma once
#include "bepch.h"

namespace Ballistic {

    struct RenderCommand;
    struct TextureHandle;
    
    class RenderDevice {
    public:
        virtual ~RenderDevice() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void Execute(const std::vector<RenderCommand>& commands) = 0;
        
        virtual void Clear(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0) = 0;
        virtual void BlitToScreen() = 0;

        virtual void Resize(uint32_t w = 0, uint32_t h = 0) = 0;

        virtual void* GetNativeTextureHandle() = 0;
    };

}