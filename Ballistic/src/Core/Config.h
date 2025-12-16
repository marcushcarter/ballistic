#pragma once

namespace Ballistic::Config {
    extern std::filesystem::path EXECUTABLE_DIR;
    extern std::filesystem::path RESOURCES_PATH;

    inline void Init(const std::filesystem::path& exeDir) {
        EXECUTABLE_DIR = exeDir;
        RESOURCES_PATH = exeDir / "../../res/";
    }

}