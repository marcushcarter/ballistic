#include "GameApplication.h"

void GameApplication::OnInit()
{
    window.onFramebufferResize = [this](uint32_t w, uint32_t h) {
        renderer.RequestWindowResize(w, h);
        renderer.RequestSceneResize(w, h);
    };
    
    renderer.onSwapchainPass = [this](VkCommandBuffer cmd) {
        VKViewportScissor(cmd, 0, 0, static_cast<float>(renderer.swapchain.extent.width), static_cast<float>(renderer.swapchain.extent.height));
        renderer.blitPipeline.Bind(cmd);
        renderer.blitPipeline.DescriptorSets(cmd, { renderer.finalImageInputSet.Get() });
        vkCmdDraw(cmd, 3, 1, 0, 0);
    };
    
    // window.SetFullscreen(true);
    
    LOG_DEBUG("Game initialized");
}

void GameApplication::OnUpdate()
{

}

void GameApplication::OnShutdown()
{
    LOG_DEBUG("Game shutdown");
}
