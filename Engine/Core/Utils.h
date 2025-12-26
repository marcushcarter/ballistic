#pragma once
#include "bepch.h"

namespace Ballistic {

    inline std::string ReadFile(const std::string& path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file)
            return {};

        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

}