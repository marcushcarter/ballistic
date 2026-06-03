#pragma once
#include "pch.h"

bool LaunchProcess(const std::filesystem::path& exePath, const std::filesystem::path& workingDir, const std::wstring& args = L"");
