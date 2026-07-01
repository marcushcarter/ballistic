#include <core/dev_tools/renderbuffer_xray.h>
#include <core/rendering/render_graph.h>
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/log/error_macros.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace ballistic {

static const uint64_t BACKBUFFER_ID = RenderGraph::intern("backbuffer");

Error RenderBufferXray::init(drivers::RenderingDeviceDriverVulkan& p_device_driver)
{
    using enum Error;
    device_driver = &p_device_driver;

    drivers::RenderingDeviceDriverVulkan::SamplerDesc sampler_desc{};
    sampler_desc.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_desc.address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler = device_driver->sampler_create(sampler_desc);

    // selected_name_id = RenderGraph::intern("final_image")

    return Ok;
}

void RenderBufferXray::shutdown()
{
    if (pending_free) {
        ImGui_ImplVulkan_RemoveTexture(pending_free);
        pending_free = VK_NULL_HANDLE;
    }
    if (set) {
        ImGui_ImplVulkan_RemoveTexture(set);
        set = VK_NULL_HANDLE;
    }
    built_for_view = VK_NULL_HANDLE;

    device_driver->sampler_free(sampler);
}

void RenderBufferXray::draw(RenderGraph& graph, uint64_t frame, uint32_t frame_count)
{
    if (!open) return;

    if (pending_free && frame >= pending_free_frame) {
        ImGui_ImplVulkan_RemoveTexture(pending_free);
        pending_free = VK_NULL_HANDLE;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(750, 400), ImGuiCond_FirstUseEver);
    bool visible = ImGui::Begin("RenderBuffer XRay - Press 2 for a screenshot of the selected buffer", &open);
    ImGui::PopStyleVar();

    if (visible) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float right_w = avail.x * 0.25f;
        float left_w = avail.x - right_w;

        VkImageView sel_view = VK_NULL_HANDLE;
        ImageResource* sel = graph.resource_by_id(selected_name_id);
        if (sel && sel->image) sel_view = sel->image->image_view;

        if (sel_view != built_for_view) {
            if (set) { pending_free = set; pending_free_frame = frame + frame_count; set = VK_NULL_HANDLE; }
            if (sel_view != VK_NULL_HANDLE) set = ImGui_ImplVulkan_AddTexture(sampler, sel_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            built_for_view = sel_view;
        }

        ImGui::BeginChild("##xray_image", ImVec2(left_w, avail.y), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImVec2 img_size = ImGui::GetContentRegionAvail();
            ImVec2 img_pos = ImGui::GetCursorScreenPos();
            if (img_size.x < 1.0f) img_size.x = 1.0f;
            if (img_size.y < 1.0f) img_size.y = 1.0f;

            if (set) {
                float tex_w = (float)sel->image->extent.width;
                float tex_h = (float)sel->image->extent.height;
                ImVec2 uv0(0.0f, 0.0f), uv1(1.0f, 1.0f);

                if (tex_w > 0.0f && tex_h > 0.0f) {
                    float pane_aspect = img_size.x / img_size.y;
                    float tex_aspect = tex_w / tex_h;
                    if (pane_aspect > tex_aspect) {
                        float u = tex_aspect / pane_aspect;
                        uv0.x = (1.0f - u) * 0.5f;
                        uv1.x = 1.0f - uv0.x;
                    } else {
                        float v = pane_aspect / tex_aspect;
                        uv0.y = (1.0f - v) * 0.5f;
                        uv1.y = 1.0f - uv0.y;
                    }
                }

                ImGui::Image((ImTextureID)set, img_size, uv0, uv1);
                
                ImVec2 mouse = ImGui::GetMousePos();
                bool hovered = mouse.x >= img_pos.x && mouse.x < img_pos.x + img_size.x && mouse.y >= img_pos.y && mouse.y < img_pos.y + img_size.y;
                if (hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Mouse: (%.0f, %.0f)", mouse.x - img_pos.x, mouse.y - img_pos.y);
                    ImGui::EndTooltip();
                }
            } else {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(img_pos, ImVec2(img_pos.x + img_size.x, img_pos.y + img_size.y), IM_COL32(25, 25, 25, 255));
            }
        }
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 0.0f);

        ImGui::BeginChild("##xray_list", ImVec2(right_w, avail.y), true);
        {
            bool any = false;
            for (const ImageResource& r : graph.image_resources) {
                if (r.name_id == BACKBUFFER_ID) continue;
                any = true;
                const std::string& name = graph.debug_names[r.name_id];
                bool is_selected = (r.name_id == selected_name_id);
                if (ImGui::Selectable(name.c_str(), is_selected)) selected_name_id = r.name_id;
            }
            if (!any) ImGui::TextDisabled("(no inspectable resources)");
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

}