#include "serialization.h"
#include "project.h"

// #define RG_IMAGE_FORMATS \
//     X(VK_FORMAT_R8G8B8A8_UNORM) \
//     X(VK_FORMAT_R8G8B8A8_SRGB) \
//     X(VK_FORMAT_B8G8R8A8_UNORM) \
//     X(VK_FORMAT_R16G16B16A16_SFLOAT) \
//     X(VK_FORMAT_R32G32B32A32_SFLOAT) \
//     X(VK_FORMAT_R32G32_SFLOAT) \
//     X(VK_FORMAT_R16G16_SFLOAT) \
//     X(VK_FORMAT_R32_SFLOAT) \
//     X(VK_FORMAT_R8_UNORM) \
//     X(VK_FORMAT_D32_SFLOAT) \
//     X(VK_FORMAT_D16_UNORM) \
//     X(VK_FORMAT_X8_D24_UNORM_PACK32) \
//     X(VK_FORMAT_D16_UNORM_S8_UINT) \
//     X(VK_FORMAT_D24_UNORM_S8_UINT) \
//     X(VK_FORMAT_D32_SFLOAT_S8_UINT)

// std::string FormatToString(VkFormat format)
// {
//     switch (format) {
//         #define X(f) case f: return #f;
//         RG_IMAGE_FORMATS
//         #undef X
//         default: return "VK_FORMAT_R8G8B8A8_UNORM";
//     }
// }

// VkFormat StringToFormat(const std::string& s)
// {
//     #define X(f) if (s == #f) return f;
//     RG_IMAGE_FORMATS
//     #undef X
//     return VK_FORMAT_R8G8B8A8_UNORM;
// }

bool Serialize(const Project& project)
{
    if (project.path.empty()) return false;
 
    try {
        toml::table root;
        root.insert("name", project.name);
        root.insert("engine_version", project.engineVersion);

        std::ofstream f(project.path / "project.blst");
        f << root;
        return true;
    } catch (...) {
        LOG_ERROR("Failed to save project.blst");
        return false;
    }
}

bool Deserialize(Project& project)
{
    const std::filesystem::path blstPath = project.path / "project.blst";
    if (!std::filesystem::exists(blstPath)) {
        LOG_ERROR("project.blst not found: %s", blstPath.string().c_str());
        return false;
    }
 
    try {
        auto root = toml::parse_file(blstPath.string());
        project.name = root["name"].value_or<std::string>("");
        project.engineVersion = root["engine_version"].value_or<std::string>("");
 
        return true;
    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse project.blst: %s", e.what());
        return false;
    }
}
