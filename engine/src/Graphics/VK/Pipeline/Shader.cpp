#include "Shader.h"

bool Shader::Compile(VkDevice device, VkShaderStageFlagBits inStage, const std::vector<uint32_t>& code)
{
    // VK_CHECK_HANDLE(device, VkDevice, false);
    // CHECK_PTR(code.data(), "SPIR-V code is empty", false);

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

void Shader::Destroy()
{
    if (shader != VK_NULL_HANDLE) {
        vkDestroyShaderModule(deviceHandle, shader, nullptr);
        shader = VK_NULL_HANDLE;
        deviceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Shader destroyed");
    }
}
