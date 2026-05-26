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