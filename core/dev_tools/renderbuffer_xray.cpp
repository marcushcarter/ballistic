#include <core/dev_tools/renderbuffer_xray.h>
#include <core/rendering/render_graph.h>
#include <core/dev_tools/imgui_texture_cache.h>
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/log/log.h>
#include <imgui.h>

namespace ballistic {

void RenderBufferXray::draw(RenderGraph& graph, ImGuiTextureCache& cache)
{
    if (!open) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(750, 400), ImGuiCond_FirstUseEver);
    bool visible = ImGui::Begin("RenderBuffer XRay - Press 2 for a screenshot of the selected buffer", &open);
    ImGui::PopStyleVar();

    if (visible) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float right_w = avail.x * 0.25f;
        float left_w = avail.x - right_w;
        
        RenderGraph::ImageResource* sel = graph.image_resource_by_id(selected_name_id);
        VkImageView sel_view = (sel && sel->image) ? sel->image->image_view : VK_NULL_HANDLE;
        VkDescriptorSet set = cache.get(sel_view);

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
            for (const RenderGraph::ImageResource& r : graph.image_resources) {
                if (r.name_id == graph.intern("backbuffer")) continue;
                if (!r.read) continue;
                if (!r.image) continue;

                if (r.kind == RenderGraph::ResourceKind::Transient && r.image_create_info.sizing != drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative)
                    continue;

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