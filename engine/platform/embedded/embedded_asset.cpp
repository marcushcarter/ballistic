#include <embedded/embedded_asset.h>
#include <windows.h>

namespace Ballistic
{
static const void* FindEmbedded(const char* name, size_t& outSize)
{
    HMODULE mod = GetModuleHandleW(nullptr);
    HRSRC res = FindResourceA(mod, name, RT_RCDATA);
    if (!res) return nullptr;

    HGLOBAL handle = LoadResource(mod, res);
    if (!handle) return nullptr;

    outSize = SizeofResource(mod, res);
    return LockResource(handle);
}

std::string EmbeddedAsset::LoadText(const char* resourceName)
{
    size_t size = 0;
    const void* data = FindEmbedded(resourceName, size);
    if (!data) return {};
    return std::string((const char*)data, size);
}

std::vector<uint8_t> EmbeddedAsset::LoadBinary(const char* resourceName)
{
    size_t size = 0;
    const void* data = FindEmbedded(resourceName, size);
    if (!data) return {};
    const uint8_t* bytes = (const uint8_t*)data;
    return std::vector<uint8_t>(bytes, bytes + size);
}
}