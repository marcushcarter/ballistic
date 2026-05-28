#pragma once
#include "pch.h"

struct AppDataPaths
{
    std::filesystem::path roamingRoot;
    std::filesystem::path localRoot;
};

AppDataPaths ResolveAppDataPaths();