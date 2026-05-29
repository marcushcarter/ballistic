#include "project.h"
#include "serialization.h"

bool Project::Create(const std::filesystem::path& projectFolder, const std::string& projectName, bool initGit)
{
    name = projectName;
    engineVersion = std::format("{}.{}.{}", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
    path = projectFolder;

    images.push_back({ GenerateID(), "GBuffer_Albedo",   VK_FORMAT_R8G8B8A8_UNORM,       ImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Normal",   VK_FORMAT_R16G16B16A16_SFLOAT,  ImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Material", VK_FORMAT_R8G8B8A8_UNORM,       ImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    images.push_back({ GenerateID(), "GBuffer_Depth",    VK_FORMAT_D32_SFLOAT,           ImageSizeMode::ViewportRelative, 1.0f, 1.0f });
    // images.push_back({ GenerateID(), "ShadowMap",        VK_FORMAT_D32_SFLOAT,           ImageSizeMode::Fixed,            0.0f, 0.0f, 2048, 2048 });
    images.push_back({ GenerateID(), "HdrLight",         VK_FORMAT_R16G16B16A16_SFLOAT,  ImageSizeMode::ViewportRelative, 1.0f, 1.0f });

    ResourceImageDesc shadow{ GenerateID(), "ShadowMap", VK_FORMAT_D32_SFLOAT };
    shadow.sizeMode = ImageSizeMode::Fixed;
    shadow.fixedWidth = 2048;
    shadow.fixedHeight = 2048;
    images.push_back(shadow);

    std::filesystem::create_directories(projectFolder);
    // std::filesystem::create_directories(projectFolder / "Assets" / "Textures");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Models");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Audio");
    // std::filesystem::create_directories(projectFolder / "Assets" / "Shaders");
    // std::filesystem::create_directories(projectFolder / "Scenes");
    std::filesystem::create_directories(projectFolder / ".ballistic");
    SetFileAttributesA((projectFolder / ".ballistic").string().c_str(), FILE_ATTRIBUTE_HIDDEN);

    if (!Save()) return false;

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
    path = projectPath;
    if (!Deserialize(*this)) {
        path.clear();
        return false;
    }
    LOG_DEBUG("Loaded project: %s (%s)", name.c_str(), path.string().c_str());
    return true;
}

bool Project::Save()
{
    if (path.empty()) return false;
    if (!Serialize(*this)) return false;
    LOG_DEBUG("Project saved: %s", path.string().c_str());
    return true;
}

void Project::Close()
{
    name.clear();
    engineVersion.clear();
    path.clear();
    images.clear();
    passes.clear();
    graph.passOrder.clear();
    LOG_DEBUG("Project closed");
}
