#include <editor/panels/viewport_panel.h>
#include <editor/editor_context.h>
#include <core/rendering/renderer.h>
#include <core/dev_tools/dev_systems.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

void ViewportPanel::draw(EditorContext& ctx)
{
    if (!open) return;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool visible = ImGui::Begin("Viewport", &open);
    ImGui::PopStyleVar();

    if (visible) {
        
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        if (size.x < 1.0f) size.x = 1.0f;
        if (size.y < 1.0f) size.y = 1.0f;
        uint32_t w = (uint32_t)size.x;
        uint32_t h = (uint32_t)size.y;

        if ((w != last_viewport_w || h != last_viewport_h) && !ImGui::IsAnyItemActive()) {
            last_viewport_w = w;
            last_viewport_h = h;

            VkImageView old_view = ctx.renderer->final_image.image_view;
            ctx.renderer->set_size(w, h);
            ctx.dev->texture_cache.retire(old_view, ctx.renderer->frame_number, ctx.renderer->frame_count);
        }

        VkDescriptorSet set = ctx.dev->texture_cache.get(ctx.renderer->final_image.image_view);

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
        if (ImGui::Button(ICON_FA_ELLIPSIS, button_size)) {}
    }
    ImGui::End();
}

}