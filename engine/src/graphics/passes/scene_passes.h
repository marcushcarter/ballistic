#pragma once
#include <graphics/passes/main_z_pass.h>
#include <graphics/passes/main_gbuffer_pass.h>
#include <graphics/passes/hbao_pass.h>
#include <graphics/passes/hbao_blur_pass.h>
#include <graphics/passes/light_pass.h>
#include <graphics/passes/sss_pass.h>
#include <graphics/passes/composite_pass.h>
#include <graphics/passes/placeholder_pass.h>
#include <graphics/passes/overlay_pass.h>
#include <core/log.h>

struct Renderer;
struct RenderGraph;
struct FrameGraph;

struct ScenePasses
{
    MainZPass mainZPass;
    MainGBufferPass mainGBufferPass;
    HBAOPass hbaoPass;
    HBAOBlurPass hbaoBlurPass;
    LightPass lightPass;
    SSSPass sssPass;
    CompositePass compositePass;
    PlaceholderPass placeholderPass;
    OverlayPass overlayPass;

    bool CreateResources(Renderer& r) {
        mainZPass.CreateResources(r);
        mainGBufferPass.CreateResources(r);
        hbaoPass.CreateResources(r);
        hbaoBlurPass.CreateResources(r);
        lightPass.CreateResources(r);
        sssPass.CreateResources(r);
        compositePass.CreateResources(r);
        placeholderPass.CreateResources(r);
        overlayPass.CreateResources(r);
        return true;
    }

    void DestroyResources() {
        mainZPass.DestroyResources();
        mainGBufferPass.DestroyResources();
        hbaoPass.DestroyResources();
        hbaoBlurPass.DestroyResources();
        lightPass.DestroyResources();
        sssPass.DestroyResources();
        compositePass.DestroyResources();
        placeholderPass.DestroyResources();
        overlayPass.DestroyResources();
    }
    
    void Build(RenderGraph& g, FrameGraph& fg) {
        mainZPass.AddPass(g, fg);
        mainGBufferPass.AddPass(g, fg);

        // cascadedshadowmaps
        // spotlightshadowmaps

        hbaoPass.AddPass(g, fg);
        hbaoBlurPass.AddPass(g, fg);

        lightPass.AddPass(g, fg);
        sssPass.AddPass(g, fg);
        compositePass.AddPass(g, fg);

        // skyandfog
        // haircoverage (depth testing)
        // maintransparentdepth
        // maintransparent
        // fgtransparent
        // finalpost
        
        placeholderPass.AddPass(g, fg);
        overlayPass.AddPass(g, fg);
        
        // upsampling
    }
};
