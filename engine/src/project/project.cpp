#include "project.h"

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

static std::string FormatToString(VkFormat format)
{
    switch (format) {
        #define X(f) case f: return #f;
        RG_IMAGE_FORMATS
        #undef X
        default: return "VK_FORMAT_R8G8B8A8_UNORM";
    }
}

static VkFormat StringToFormat(const std::string& s)
{
    #define X(f) if (s == #f) return f;
    RG_IMAGE_FORMATS
    #undef X
    return VK_FORMAT_R8G8B8A8_UNORM;
}

bool Project::Create(const std::filesystem::path& projectFolder, const std::string& projectName, bool initGit)
{
    name = projectName;
    engineVersion = std::format("{}.{}.{}", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
    path = projectFolder;

    images.push_back({ GenerateID(), "GBuffer_Albedo",   VK_FORMAT_R8G8B8A8_UNORM,       RGImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Normal",   VK_FORMAT_R16G16B16A16_SFLOAT,  RGImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Material", VK_FORMAT_R8G8B8A8_UNORM,       RGImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Depth",    VK_FORMAT_D32_SFLOAT,           RGImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "ShadowMap",        VK_FORMAT_D32_SFLOAT,           RGImageSizeMode::Fixed,            0.0f, 0.0f, 2048, 2048 });
    images.push_back({ GenerateID(), "HdrLight",         VK_FORMAT_R16G16B16A16_SFLOAT,  RGImageSizeMode::ViewportRelative, 1.0f, 1.0f });

    std::filesystem::create_directories(projectFolder);
    // std::filesystem::create_directories(projectFolder / "Assets" / "Textures");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Models");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Audio");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Shaders");
    // std::filesystem::create_directories(projectFolder / "Scenes");
    std::filesystem::create_directories(projectFolder / ".ballistic");
    SetFileAttributesA((projectFolder / ".ballistic").string().c_str(), FILE_ATTRIBUTE_HIDDEN);

    Save();

    if (initGit) {
        std::ofstream gitignore(projectFolder / ".gitignore");
        gitignore << "# Ballistic Engine\n";
        gitignore << ".ballistic/\n";

        std::ofstream gitattributes(projectFolder / ".gitattributes");
        gitattributes << "* text=auto eol=lf\n\n";
        gitattributes << "# Binary assets\n";
        gitattributes << "*.png binary\n";
        gitattributes << "*.jpg binary\n";
        gitattributes << "*.jpeg binary\n";
        gitattributes << "*.tga binary\n";
        gitattributes << "*.wav binary\n";
        gitattributes << "*.mp3 binary\n";
        gitattributes << "*.ogg binary\n";
        gitattributes << "*.fbx binary\n";
        gitattributes << "*.blend binary\n\n";
        gitattributes << "# Engine files\n";
        gitattributes << "*.blst text eol=lf\n";
        gitattributes << "*.glsl text eol=lf\n";
        gitattributes << "*.hlsl text eol=lf\n";
    }

    LOG_INFO("Project created: %s", projectFolder.string().c_str());
    return true;
}

bool Project::Load(const std::filesystem::path& projectPath)
{
    if (!std::filesystem::exists(projectPath / "project.blst")) {
        LOG_ERROR("project.blst not found: %s", projectPath.string().c_str());
        return false;
    }

    try {
        auto blst = toml::parse_file((projectPath / "project.blst").string());
        name = blst["name"].value_or<std::string>("");
        engineVersion = blst["engine_version"].value_or<std::string>("");
        path = projectPath;
    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse project.blst: %s", e.what());
        return false;
    }

    if (std::filesystem::exists(projectPath / "render_graph.blst")) {
        try {
            auto rg = toml::parse_file((projectPath / "render_graph.blst").string());
            auto arr = rg["images"].as_array();
            if (arr) {
                arr->for_each([&](auto& el) {
                    if (!el.is_table()) return;
                    auto& t = *el.as_table();

                    RGImage img;
                    img.id = t["id"].value_or<uint64_t>(0);
                    img.name = t["name"].value_or<std::string>("");
                    img.format = StringToFormat(t["format"].value_or<std::string>("VK_FORMAT_R8G8B8A8_UNORM"));
                    img.sizeMode = t["size_mode"].value_or<std::string>("") == "fixed" ? RGImageSizeMode::Fixed : RGImageSizeMode::ViewportRelative;
                    img.relativeWidth = (float)t["relative_width"].value_or<double>(1.0);
                    img.relativeHeight = (float)t["relative_height"].value_or<double>(1.0);
                    img.fixedWidth = t["fixed_width"].value_or<uint32_t>(0);
                    img.fixedHeight = t["fixed_height"].value_or<uint32_t>(0);
                    img.usageAttachment = t["usage_attachment"].value_or(true);
                    img.usageSampled = t["usage_sampled"].value_or(true);
                    img.usageStorage = t["usage_storage"].value_or(false);

                    if (img.id != 0 && !img.name.empty()) images.push_back(img);
                });
            }
        } catch (const toml::parse_error& e) {
            LOG_ERROR("Failed to parse render_graph.blst: %s", e.what());
        }
    }

    LOG_DEBUG("Loaded project: %s (%s)", name.c_str(), path.string().c_str());
    return true;
}

bool Project::Save()
{
    if (path.empty()) return false;

    try {
        toml::table blst;
        blst.insert("name", name);
        blst.insert("engine_version", engineVersion);
        std::ofstream f(path / "project.blst");
        f << blst;
    } catch (...) {
        LOG_ERROR("Failed to save project.blst");
        return false;
    }

    try {
        toml::table rg;
        toml::array arr;
        for (auto& img : images) {
            toml::table entry;
            entry.insert("id", (int64_t)img.id);
            entry.insert("name", img.name);
            entry.insert("format", FormatToString(img.format));
            entry.insert("size_mode", img.sizeMode == RGImageSizeMode::Fixed ? "fixed" : "viewport_relative");
            entry.insert("relative_width", (double)img.relativeWidth);
            entry.insert("relative_height", (double)img.relativeHeight);
            entry.insert("fixed_width", (int64_t)img.fixedWidth);
            entry.insert("fixed_height", (int64_t)img.fixedHeight);
            entry.insert("usage_attachment", img.usageAttachment);
            entry.insert("usage_sampled", img.usageSampled);
            entry.insert("usage_storage", img.usageStorage);
            arr.push_back(entry);
        }
        rg.insert("images", arr);
        std::ofstream f(path / "render_graph.blst");
        f << rg;
    } catch (...) {
        LOG_ERROR("Failed to save render_graph.blst");
        return false;
    }

    LOG_DEBUG("Project saved: %s", path.string().c_str());
    return true;
}

void Project::Close()
{
    name.clear();
    engineVersion.clear();
    path.clear();
    images.clear();
    LOG_DEBUG("Project closed");
}
