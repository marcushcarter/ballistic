#include "GameApplication.h"

void GameApplication::OnInit()
{
    LOG_DEBUG("Game initialized");
    // window.SetFullscreen(true);
    
    renderer.onSwapchainPass = [this](VkCommandBuffer cmd) {
        (void)cmd;
        // VKViewportScissor(cmd, 0, 0, renderer.swapchain.extent.width, renderer.swapchain.extent.height);
        // renderer.blitPipeline.Bind(cmd);
        // renderer.blitPipeline.DescriptorSets(cmd, { renderer.finalImageInputSet.Get() });
        // vkCmdDraw(cmd, 3, 1, 0, 0);
    };
}

void GameApplication::OnUpdate()
{

}

void GameApplication::OnShutdown()
{
    LOG_DEBUG("Game shutdown");
}
