#pragma once
#include "pch.h"

inline static std::vector<uint32_t> LoadSPV(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_DEBUG("[VULKAN] Failed to open SPV file: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        LOG_DEBUG("[VULKAN] Invalid SPV file size: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    if (size % 4 != 0) {
        LOG_DEBUG("[VULKAN] SPV file size not multiple of 4 bytes: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    std::vector<uint32_t> out(size / 4);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(out.data()), size)) {
        LOG_DEBUG("[VULKAN] Failed reading SPV file: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    return out;
}

inline std::vector<uint32_t> LoadEmbeddedSPV(int resourceID)
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    HGLOBAL mem = LoadResource(nullptr, res);
    void* data = LockResource(mem);
    DWORD size = SizeofResource(nullptr, res);

    std::vector<uint32_t> spv(size / 4);
    memcpy(spv.data(), data, size);
    return spv;
}

struct Shader
{
    VkShaderModule shader = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;
    std::vector<uint32_t> spirv;
    
    bool Compile(VkDevice device, VkShaderStageFlagBits stage, const std::vector<uint32_t>& code);
    // bool CompileGLSL(VkDevice device, VkShaderStageFlagBits shaderStage, const char* source, const char* name);
    void Destroy();
    
    VkShaderModule Get() const { return shader; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
};