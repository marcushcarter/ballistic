#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>
#include <vector>
#include <string>

std::vector<uint32_t> LoadSPV(const std::filesystem::path& path);
std::string LoadShaderSource(int resourceID);

struct Shader
{
    VkShaderModule shader = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage;

    std::vector<uint32_t> spirv;
    std::filesystem::path cachePath;
    
    bool Compile(VkDevice device, VkShaderStageFlagBits stage, const std::vector<uint32_t>& code);
    bool LoadOrCompile(VkDevice device, VkShaderStageFlagBits stage, const std::string& glsl, const std::filesystem::path& cacheFile, const char* name);
    void Destroy();

    VkShaderModule Get() const { return shader; }
    
private:
    VkDevice deviceHandle = VK_NULL_HANDLE;
    
    bool LoadFromCache(const std::filesystem::path& path);
    bool SaveToCache(const std::filesystem::path& path);
    bool CompileFromGLSL(const std::string& glsl, const char* name);
    
    bool CreateModule(VkDevice device);
};