#pragma once
#include "pch.h"

struct Project;
struct Renderer;

struct ProjectLoader
{
    enum class Status { Idle, Loading, Succeeded, Failed };
 
    std::future<void> future;
    std::atomic<bool> done{false};
    std::atomic<bool> failed{false};

    void Begin(Project& project, const std::filesystem::path& path);
    Status Poll() const;
    void Reset();
};