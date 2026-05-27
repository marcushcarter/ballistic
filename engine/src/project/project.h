#pragma once
#include "pch.h"

struct Project
{
    std::string name;
    std::string engineVersion;
    std::filesystem::path path;

    bool Load(const std::filesystem::path& projectPath);
    bool Save();
    void Close();

    bool IsOpen() const { return !path.empty(); }
};
