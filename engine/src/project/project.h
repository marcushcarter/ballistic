#pragma once
#include "pch.h"
#include "resource_desc.h"

struct Project
{
    std::string name;
    std::string engineVersion;
    std::filesystem::path path;

    std::vector<ResourceImageDesc> images;
    std::vector<RGPass> passes;
    RenderGraph graph;

    bool Create(const std::filesystem::path& projectFolder, const std::string& projectName, bool initGit);
    bool Load(const std::filesystem::path& projectPath);
    bool Save();
    void Close();

    bool IsOpen() const { return !path.empty(); }
};
