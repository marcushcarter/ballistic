#include <pipeline/shader.h>
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <windows.h>

namespace Ballistic::Vulkan
{
std::vector<uint32_t> LoadSPV(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // LOG_DEBUG("[VULKAN] Failed to open SPV file: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        // LOG_DEBUG("[VULKAN] Invalid SPV file size: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    if (size % 4 != 0) {
        // LOG_DEBUG("[VULKAN] SPV file size not multiple of 4 bytes: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    std::vector<uint32_t> out(size / 4);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(out.data()), size)) {
        // LOG_DEBUG("[VULKAN] Failed reading SPV file: %s", std::filesystem::absolute(path).string().c_str());
        return {};
    }
    return out;
}

std::string LoadShaderSource(int resourceID)
{
    HRSRC res = FindResource(nullptr, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!res) { /* LOG_ERROR("LoadShaderSource: resource %d not found", resourceID); */ return {}; }
    HGLOBAL mem = LoadResource(nullptr, res);
    const char* data = (const char*)LockResource(mem);
    DWORD size = SizeofResource(nullptr, res);
    if (!data || size == 0) { /* LOG_ERROR("LoadShaderSource: resource %d empty", resourceID); */ return {}; }
    return std::string(data, size);
}

static shaderc_shader_kind ToKind(VkShaderStageFlagBits s)
{
    switch (s) {
        case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_vertex_shader;
        case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_fragment_shader;
        case VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_compute_shader;
        case VK_SHADER_STAGE_GEOMETRY_BIT: return shaderc_geometry_shader;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_tess_control_shader;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_tess_evaluation_shader;
        default: return shaderc_glsl_infer_from_source;
    }
}

static size_t HashGLSL(const std::string& glsl) { return std::hash<std::string>{}(glsl); }

bool Shader::Compile(VkDevice device, VkShaderStageFlagBits inStage, const std::vector<uint32_t>& code)
{
    Destroy();
    stage = inStage;
    spirv = code;
    deviceHandle = device;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS) {
        // LOG_ERROR("Failed to create Vulkan shader module");
        return false;
    }

    return true;
}

bool Shader::LoadOrCompile(VkDevice device, VkShaderStageFlagBits inStage, const std::string& glsl, const std::filesystem::path& cacheFile, const char* name)
{
    Destroy();
    stage = inStage;
    cachePath = cacheFile;
    deviceHandle = device;

    std::filesystem::create_directories(cacheFile.parent_path());

    size_t newHash = HashGLSL(glsl);
    size_t oldHash = 0;
    bool hashValid = false;

    std::ifstream hashFileLoad(cacheFile.string() + ".hash", std::ios::binary);
    if (hashFileLoad.is_open()) {
        hashFileLoad.read(reinterpret_cast<char*>(&oldHash), sizeof(oldHash));
        hashValid = (oldHash == newHash);
    }

    if (hashValid && LoadFromCache(cacheFile)) {
        if (CreateModule(device)) return true;
    }

    if (!CompileFromGLSL(glsl, name)) return false;
    if (!CreateModule(device)) return false;
    SaveToCache(cacheFile);

    std::ofstream hashFileSave(cacheFile.string() + ".hash", std::ios::binary);
    if (hashFileSave.is_open()) {
        hashFileSave.write(reinterpret_cast<const char*>(&newHash), sizeof(newHash));
    }

    return true;
}

void Shader::Destroy()
{
    if (shader) {
        vkDestroyShaderModule(deviceHandle, shader, nullptr);
        shader = VK_NULL_HANDLE;
    }
}

bool Shader::LoadFromCache(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    const size_t size = static_cast<size_t>(file.tellg());
    if (size == 0 || size % 4 != 0) return false;
    spirv.resize(size / 4);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(spirv.data()), size)) return false;
    return true;
}

bool Shader::SaveToCache(const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
    return true;
}

bool Shader::CompileFromGLSL(const std::string& glsl, const char* name)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    auto result = compiler.CompileGlslToSpv(glsl, ToKind(stage), name, options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        // LOG_ERROR("Shader compile failed: %s\n%s", name, result.GetErrorMessage().c_str());
        return false;
    }
    spirv = { result.cbegin(), result.cend() };
    return true;
}

bool Shader::CreateModule(VkDevice device)
{
    deviceHandle = device;

    if (spirv.empty()) {
        // LOG_ERROR("Empty SPIR-V");
        return false;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS) {
        // LOG_ERROR("Shader: vkCreateShaderModule failed");
        return false;
    }

    return true;
}
}