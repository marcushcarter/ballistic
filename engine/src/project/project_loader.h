#pragma once
#include "pch.h"

struct Project;
struct Renderer;

struct ProjectLoader
{
    std::future<void> loadFuture;
    std::atomic<bool> loading{false};
    std::atomic<bool> dataReady{false};
    std::atomic<bool> failed{false};

    void Begin(Project& project, const std::filesystem::path& path);

    bool Poll();
};