#pragma once
#include "pch.h"
#include "render_graph.h"

struct Project
{
    std::string name;
    std::string engineVersion;
    std::filesystem::path path;

    std::vector<RGImage> images;
    // std::vector<RenderGraph> graphs;

    bool Create(const std::filesystem::path& projectFolder, const std::string& projectName, bool initGit);
    bool Load(const std::filesystem::path& projectPath);
    bool Save();
    void Close();

    bool IsOpen() const { return !path.empty(); }
};
