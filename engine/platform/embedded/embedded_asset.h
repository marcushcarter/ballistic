#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Ballistic
{
struct EmbeddedAsset
{
    static std::string LoadText(const char* resourceName);
    static std::vector<uint8_t> LoadBinary(const char* resourceName);
};
}