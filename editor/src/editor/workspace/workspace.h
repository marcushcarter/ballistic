#pragma once
#include "pch.h"
#include "project_registry.h"

struct EditorConfig
{
    bool autosaveEnabled = true;
    float autosaveInterval = 120.0f;
};

struct EditorWorkspace
{
    std::filesystem::path roamingRoot;
    std::filesystem::path localRoot;

    ProjectRegistry registry;
    EditorConfig config;

    void Load();
    void Save();

    void TouchProject(const std::filesystem::path& projectPath);
};