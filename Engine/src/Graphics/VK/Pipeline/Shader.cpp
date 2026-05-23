#include "Shader.h"

bool Shader::Compile(VkDevice device, VkShaderStageFlagBits inStage, const std::vector<uint32_t>& code)
{
    VK_CHECK_HANDLE(device, VkDevice);
    CHECK_PTR(code.data(), "SPIR-V code is empty");

    Destroy();
    stage = inStage;
    spirv = code;
    deviceHandle = device;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan shader module");
        return false;
    }

    LOG_DEBUG("Shader created");
    return true;
}

// bool Shader::CompileGLSL(VkDevice device, VkShaderStageFlagBits shaderStage, const char* source, const char* name)
// {
//     shaderc_shader_kind kind;
//     switch (shaderStage) {
//         case VK_SHADER_STAGE_VERTEX_BIT:   kind = shaderc_vertex_shader;   break;
//         case VK_SHADER_STAGE_FRAGMENT_BIT: kind = shaderc_fragment_shader; break;
//         case VK_SHADER_STAGE_COMPUTE_BIT:  kind = shaderc_compute_shader;  break;
//         default:
//             LOG_ERROR("Unsupported shader stage");
//             return false;
//     }

//     shaderc::Compiler compiler;
//     shaderc::CompileOptions options;
//     options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
//     options.SetOptimizationLevel(shaderc_optimization_level_performance);

//     shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, strlen(source), kind, name, options);

//     if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
//         LOG_ERROR("Shader compile error [%s]: %s", name, result.GetErrorMessage().c_str());
//         return false;
//     }

//     std::vector<uint32_t> spv(result.cbegin(), result.cend());
//     return Compile(device, shaderStage, spv);
// }

void Shader::Destroy()
{
    if (shader != VK_NULL_HANDLE) {
        vkDestroyShaderModule(deviceHandle, shader, nullptr);
        shader = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Shader destroyed");
    }
}
