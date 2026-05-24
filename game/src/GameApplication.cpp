#include "GameApplication.h"

void GameApplication::OnInit()
{
    // window.SetFullscreen(true);
    
    renderer.onSwapchainPass = [this](VkCommandBuffer cmd) {
        VKViewportScissor(cmd, 0, 0, static_cast<float>(renderer.swapchain.extent.width), static_cast<float>(renderer.swapchain.extent.height));
        renderer.blitPipeline.Bind(cmd);
        renderer.blitPipeline.DescriptorSets(cmd, { renderer.finalImageInputSet.Get() });
        vkCmdDraw(cmd, 3, 1, 0, 0);
    };
    
    LOG_DEBUG("Game initialized");
}

void GameApplication::OnUpdate()
{

}

void GameApplication::OnShutdown()
{
    LOG_DEBUG("Game shutdown");
}
