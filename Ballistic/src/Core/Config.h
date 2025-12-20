#pragma once
#include "bepch.h"

namespace Ballistic::Config {

    extern std::filesystem::path EXECUTABLE_DIR;
    extern std::filesystem::path BALLISTIC_RES_PATH;
    extern std::filesystem::path EDITOR_RES_PATH;

    inline void Init(const std::filesystem::path& exeDir) {
        EXECUTABLE_DIR = exeDir;
        BALLISTIC_RES_PATH = exeDir / "../../../Ballistic/res/";
        EDITOR_RES_PATH = exeDir / "../../../BallisticEditor/res/";
    }

}