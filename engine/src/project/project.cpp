#include "project.h"
#include "render_graph_io.h"

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

    // uint64_t shadowPassID   = GenerateID();
    // uint64_t zprepassID     = GenerateID();
    // uint64_t gbufferPassID  = GenerateID();
    // uint64_t lightingPassID = GenerateID();
    // uint64_t postPassID     = GenerateID();

    // passes.push_back({ shadowPassID,  "ShadowPass",   "Shadow",   true, {}, { shadowMapID } });
    // passes.push_back({ zprepassID,    "ZPrepass",      "ZPrepass", true, {}, { depthID } });
    // passes.push_back({ gbufferPassID, "GBufferPass",   "GBuffer",  true, {}, { albedoID, normalID, materialID, depthID } });
    // passes.push_back({ lightingPassID,"LightingPass",  "Lighting", true, { albedoID, normalID, materialID, depthID, shadowMapID }, { hdrLightID } });
    // passes.push_back({ postPassID,    "PostProcess",   "PostProcess", true, { hdrLightID }, {} });  // writes to FinalImage implicitly

    // graph.passOrder = { shadowPassID, zprepassID, gbufferPassID, lightingPassID, postPassID };
    // graph.passOrder = { shadowPassID };

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
        LoadRenderGraph(projectPath / "render_graph.blst", images);
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

    if (!SaveRenderGraph(path / "render_graph.blst", images)) {
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
