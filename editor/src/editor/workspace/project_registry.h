#pragma once
#include "pch.h"

struct ProjectEntry
{
    std::string path;
    std::string name;
    bool favorite = false;
    std::string lastOpened;
    std::string engineVersion;
};

struct ProjectRegistry
{
    std::filesystem::path cfgPath;
    std::vector<ProjectEntry> entries;

    void Load(const std::filesystem::path& roamingRoot);
    void Save();

    ProjectEntry* Find(const std::filesystem::path& projectPath);
    void Add(const ProjectEntry& entry);
    void Remove(size_t index);
    void UpdateTimestamp(const std::filesystem::path& projectPath);
};