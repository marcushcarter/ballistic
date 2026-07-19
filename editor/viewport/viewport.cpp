#include <editor/viewport/viewport.h>
#include <core/rendering/renderer.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/dev_tools/dev_tools.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

int Viewport::push_style()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    return 1;
}

void Viewport::draw_contents(EditorContext& ctx)
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    if (!ImGui::IsAnyItemActive()) {
        ctx.renderer->request_size((uint32_t)size.x, (uint32_t)size.y);
    }
    
    RenderGraph::ImageResource* sel = ctx.renderer->graph.image_resource_by_id(selected_name_id);
    VkImageView sel_view = (sel && sel->image) ? sel->image->image_view : VK_NULL_HANDLE;
    VkDescriptorSet set = ctx.imgui->texture_cache.get(sel_view);

    if (set) {
        ImGui::Image((ImTextureID)set, size);
    } else {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(25, 25, 25, 255));
    }

    const float margin = 8.0f;
    ImVec2 button_size = ImVec2(24, 24);
    ImVec2 button_pos = ImVec2(ImGui::GetWindowContentRegionMax().x - button_size.x, ImGui::GetWindowContentRegionMin().y + margin);

    ImGui::SetCursorPos(button_pos);
    if (ImGui::Button(ICON_FA_ELLIPSIS, button_size)) {
        ImGui::OpenPopup("##viewport_source");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("##viewport_source")) {
        if (ImGui::Selectable("Final Output", selected_name_id == 0)) selected_name_id = 0;
        ImGui::Separator();

        bool any = false;
        for (const RenderGraph::ImageResource& r : ctx.renderer->graph.image_resources) {
            if (r.name_id == ctx.renderer->graph.intern("Backbuffer")) continue;
            if (r.kind == RenderGraph::ResourceKind::Transient && r.image_create_info.sizing != drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative) continue;
            if (!r.read) continue;
            if (!r.image) continue;

            any = true;
            const std::string& name = ctx.renderer->graph.debug_names[r.name_id];
            bool is_selected = (r.name_id == selected_name_id);
            if (ImGui::Selectable(name.c_str(), is_selected)) selected_name_id = r.name_id;
        }
        if (!any) ImGui::TextDisabled("(no inspectable resources)");

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

}