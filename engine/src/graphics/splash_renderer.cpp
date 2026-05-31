#include "splash_renderer.h"
#include "renderer.h"
#include "shaders.h"
#include "resources.h"
#include "graphics/render_paths/splash_render_path.h"

inline bool LoadRCImage(VkDevice device, VmaAllocator vma, VkCommandBuffer cmd, int resourceID, Image2D& outImage, Buffer& outStaging, const char* debugName = nullptr)
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!res) {
        LOG_ERROR("LoadRCImage failed: resource %d not found", resourceID);
        return false;
    }
    
    HGLOBAL mem = LoadResource(nullptr, res);
    void* data = LockResource(mem);
    DWORD size = SizeofResource(nullptr, res);

    int w, h, channels;
    stbi_uc* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &w, &h, &channels, 4);
    if (!pixels) {
        LOG_ERROR("LoadRCImage failed: stbi decode failed for resource %d", resourceID);
        return false;
    }

    VkDeviceSize imageSize = (VkDeviceSize)w * h * 4;

    if (!outImage.Create(device, vma, {
        .extent = { (uint32_t)w, (uint32_t)h },
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
        .debugName = debugName
    })) { stbi_image_free(pixels); return false; }

    if (!outStaging.Create(device, vma, {
        .size = imageSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .hostVisible = true
    })) { stbi_image_free(pixels); return false; }

    outStaging.Update(pixels, imageSize);
    stbi_image_free(pixels);

    outImage.Transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
    outImage.CopyBuffer(cmd, outStaging.Get());
    outImage.Transition(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);

    return true;
}

bool SplashRenderer::Create(Renderer& renderer)
{
    BE_ASSERT(splashSet.Allocate(renderer.device.Get(), {
        .pool = renderer.descriptorPool.Get(),
        .setLayout = renderer.imageInputSetLayout.Get(),
        .debugName = "SplashSet"
    }));

    BE_ASSERT(splashPipelineLayout.Create(renderer.device.Get(), {
        .setLayouts = { renderer.imageInputSetLayout.Get() },
        .pushConstants = { PushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)) },
        .debugName = "SplashPipelineLayout"
    }));

    Shader vert{}, frag{};
    BE_ASSERT(vert.Compile(renderer.device.Get(), VK_SHADER_STAGE_VERTEX_BIT, { SHADER_SPRITE_VERT, SHADER_SPRITE_VERT + SHADER_SPRITE_VERT_SIZE / 4 }));    
    BE_ASSERT(frag.Compile(renderer.device.Get(), VK_SHADER_STAGE_FRAGMENT_BIT, { SHADER_SPRITE_FRAG, SHADER_SPRITE_FRAG + SHADER_SPRITE_FRAG_SIZE / 4 }));

    PipelineRenderingInfo renderingInfo;
    renderingInfo.colorFormats = { renderer.swapchain.format };
    auto renderingCreateInfo = renderingInfo.Get();

    BE_ASSERT(splashPipeline.Create(renderer.device.Get(), {
        .pNext = &renderingCreateInfo,
        .layout = splashPipelineLayout.Get(),
        .shaderStages = { PipelineShaderStage(vert.Get(), vert.stage), PipelineShaderStage(frag.Get(), frag.stage) },
        .debugName = "SplashPipeline"
    }));

    vert.Destroy();
    frag.Destroy();

    CommandPool transferCommandPool;
    BE_ASSERT(transferCommandPool.Create(renderer.device.Get(), {
        .queueFamilyIndex = renderer.transferQueue.familyIndex,
        .transient = true,
        .debugName = "SplashTransferCommandPool"
    }));
    
    CommandBuffer transferCmd;
    BE_ASSERT(transferCmd.Allocate(renderer.device.Get(), transferCommandPool.Get(), false, "SplashTransferCommandBuffer"));
    transferCmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    std::vector<Buffer> stagingBuffers;
    
    Buffer& logoStaging = stagingBuffers.emplace_back();
    BE_ASSERT(LoadRCImage(renderer.device.Get(), renderer.allocator.Get(), transferCmd.Get(), IMG_LOGO_PNG, logoImage, logoStaging, "LogoImage"));

    Buffer& logoLongStaging = stagingBuffers.emplace_back();
    BE_ASSERT(LoadRCImage(renderer.device.Get(), renderer.allocator.Get(), transferCmd.Get(), IMG_LOGO_LONG_PNG, logoLongImage, logoLongStaging, "LogoLongImage"));
    
    transferCmd.End();
    renderer.transferQueue.Submit(transferCmd.Get());
    renderer.transferQueue.WaitIdle();

    for (auto& s : stagingBuffers) s.Destroy();
    transferCmd.Free();
    transferCommandPool.Destroy();
    
    splashSet.SetImages(0, { logoImage.GetView() }, renderer.linearSampler.Get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    LOG_DEBUG("Splash Renderer created");
    return true;
}

void SplashRenderer::Destroy()
{
    logoImage.Destroy();
    logoLongImage.Destroy();

    splashPipeline.Destroy();
    splashPipelineLayout.Destroy();
    
    LOG_DEBUG("Splash Renderer destroyed");
}

void SplashRenderer::RecordSplashContent(VkCommandBuffer cmd, Renderer& renderer)
{
    VkClearAttachment clearAttachment{};
    clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAttachment.colorAttachment = 0;
    clearAttachment.clearValue.color = { { 0.05f, 0.05f, 0.05f, 1.0f } };

    VkClearRect clearRect{};
    clearRect.rect.offset = { 0, 0 };
    clearRect.rect.extent = renderer.swapchain.extent;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);

    float scale  = 0.3f;
    float aspect = (float)logoImage.extent.width / (float)logoImage.extent.height;
    float imgH = scale;
    float imgW = scale * aspect * ((float)renderer.swapchain.extent.height / (float)renderer.swapchain.extent.width);
    RecordQuad(renderer, cmd, (1.0f - imgW) * 0.5f, (1.0f - imgH) * 0.5f, imgW, imgH);
}

bool SplashRenderer::RenderLoadingFrame(Renderer& renderer)
{
    SplashRenderPath path(renderer, *this);
    renderer.Render(path);
    return true;
}

void SplashRenderer::RecordQuad(Renderer& renderer, VkCommandBuffer cmd, float xNorm, float yNorm, float wNorm, float hNorm)
{
    PushConstants pc = { xNorm, yNorm, wNorm, hNorm };
    ViewportScissor(cmd, 0, 0, (float)renderer.swapchain.extent.width, (float)renderer.swapchain.extent.height);
    splashPipeline.Bind(cmd);
    splashPipeline.DescriptorSets(cmd, { splashSet.Get() });
    splashPipeline.PushConstants(cmd, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);
    vkCmdDraw(cmd, 6, 1, 0, 0);
}
