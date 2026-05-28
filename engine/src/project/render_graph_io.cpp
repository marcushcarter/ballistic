#include "render_graph_io.h"

#define RG_IMAGE_FORMATS \
    X(VK_FORMAT_R8G8B8A8_UNORM) \
    X(VK_FORMAT_R8G8B8A8_SRGB) \
    X(VK_FORMAT_B8G8R8A8_UNORM) \
    X(VK_FORMAT_R16G16B16A16_SFLOAT) \
    X(VK_FORMAT_R32G32B32A32_SFLOAT) \
    X(VK_FORMAT_R32G32_SFLOAT) \
    X(VK_FORMAT_R16G16_SFLOAT) \
    X(VK_FORMAT_R32_SFLOAT) \
    X(VK_FORMAT_R8_UNORM) \
    X(VK_FORMAT_D32_SFLOAT) \
    X(VK_FORMAT_D16_UNORM) \
    X(VK_FORMAT_X8_D24_UNORM_PACK32) \
    X(VK_FORMAT_D16_UNORM_S8_UINT) \
    X(VK_FORMAT_D24_UNORM_S8_UINT) \
    X(VK_FORMAT_D32_SFLOAT_S8_UINT)

std::string RGFormatToString(VkFormat format)
{
    switch (format) {
        #define X(f) case f: return #f;
        RG_IMAGE_FORMATS
        #undef X
        default: return "VK_FORMAT_R8G8B8A8_UNORM";
    }
}

VkFormat RGStringToFormat(const std::string& s)
{
    #define X(f) if (s == #f) return f;
    RG_IMAGE_FORMATS
    #undef X
    return VK_FORMAT_R8G8B8A8_UNORM;
}

bool LoadRenderGraph(const std::filesystem::path& blstPath, std::vector<RGImage>& outImages)
{
    // TODO: move from Project::Load
    (void) blstPath;
    (void) outImages;
    return false;
}

bool SaveRenderGraph(const std::filesystem::path& blstPath, const std::vector<RGImage>& images)
{
    // TODO: move from Project::Save
    (void) blstPath;
    (void) images;
    return false;
}