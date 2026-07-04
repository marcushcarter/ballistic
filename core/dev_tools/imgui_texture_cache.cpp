#include <core/dev_tools/imgui_texture_cache.h>
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/log/error_macros.h>
#include <backends/imgui_impl_vulkan.h>

namespace ballistic {

Error ImGuiTextureCache::create(drivers::DeviceDriverVulkan& r_device_driver)
{
    using enum Error;

    device_driver = &r_device_driver;

    drivers::DeviceDriverVulkan::SamplerCreateInfo sampler_ci{};
    sampler_ci.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_ci.address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.name = "imgui_preview_sampler";

    sampler = device_driver->sampler_create(sampler_ci);
    BALLISTIC_ERR_FAIL_COND_V(sampler == VK_NULL_HANDLE, Failed);

    return Ok;
}

void ImGuiTextureCache::destroy()
{
    invalidate_all();
    if (sampler && device_driver) device_driver->sampler_free(sampler);
}

VkDescriptorSet ImGuiTextureCache::get(VkImageView p_view)
{
    if (p_view == VK_NULL_HANDLE) return VK_NULL_HANDLE;

    auto it = entries.find(p_view);
    if (it != entries.end()) return it->second.set;

    VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(sampler, p_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    entries.emplace(p_view, Entry{ set, 0 });
    return set;
}

void ImGuiTextureCache::retire(VkImageView p_view, uint64_t p_frame_number, uint32_t p_frame_count)
{
    if (p_view == VK_NULL_HANDLE) return;
    auto it = entries.find(p_view);
    if (it == entries.end()) return;
    retired.push_back({ it->second.set, p_frame_number + p_frame_count });
    entries.erase(it);
}

void ImGuiTextureCache::collect(uint64_t p_frame_number)
{
    for (size_t i = 0; i < retired.size();) {
        if (p_frame_number >= retired[i].retire_frame) {
            ImGui_ImplVulkan_RemoveTexture(retired[i].set);
            retired[i] = retired.back();
            retired.pop_back();
        } else {
            ++i;
        }
    }
}

void ImGuiTextureCache::invalidate_all()
{
    // caller must have device_wait_idle'd
    for (Retired& r : retired) ImGui_ImplVulkan_RemoveTexture(r.set);
    for (auto& [view, e] : entries) ImGui_ImplVulkan_RemoveTexture(e.set);
    retired.clear();
    entries.clear();
}

}