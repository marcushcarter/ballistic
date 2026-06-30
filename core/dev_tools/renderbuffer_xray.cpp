#include <core/dev_tools/renderbuffer_xray.h>
#include <imgui.h>
#include <cstdint>

namespace ballistic {

void RenderBufferXray::draw()
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

        ImGui::BeginChild("##xray_image", ImVec2(left_w, avail.y), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImVec2 img_size = ImGui::GetContentRegionAvail();
            ImVec2 img_pos = ImGui::GetCursorScreenPos();
            if (img_size.x < 1.0f) img_size.x = 1.0f;
            if (img_size.y < 1.0f) img_size.y = 1.0f;

            if (false) {
                // float pane_aspect = img_size.x / img_size.y;
                // float tex_aspect = (float)tex_width / (float)tex_height;
                // ImVec2 uv0(0,0), uv1(1,1);
                // if (pane_aspect > tex_aspect) {
                //     // pane wider than texture -> crop top/bottom
                //     float v = tex_aspect / pane_aspect;
                //     uv0.y = (1.0f - v) * 0.5f;
                //     uv1.y = 1.0f - uv0.y;
                // } else {
                //     // pane taller -> crop left/right
                //     float u = pane_aspect / tex_aspect;
                //     uv0.x = (1.0f - u) * 0.5f;
                //     uv1.x = 1.0f - uv0.x;
                // }
                // ImGui::Image((ImTextureID)tex, img_size, uv0, uv1);
            } else {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(img_pos, ImVec2(img_pos.x + img_size.x, img_pos.y + img_size.y), IM_COL32(255, 255, 255, 255));

                ImVec2 mouse = ImGui::GetMousePos();
                bool hovered = mouse.x >= img_pos.x && mouse.x < img_pos.x + img_size.x && mouse.y >= img_pos.y && mouse.y < img_pos.y + img_size.y;

                if (hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Mouse: (%.0f, %.0f)", mouse.x - img_pos.x, mouse.y - img_pos.y);
                    ImGui::Separator();
                    ImGui::Text("RGBA: 255 255 255 255");
                    ImGui::Text("Depth: 1.000");
                    ImGui::EndTooltip();
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 0.0f);

        ImGui::BeginChild("##xray_list", ImVec2(right_w, avail.y), true);
        {
            // placeholder
            static int selected = -1;
            const char* entries[] = { "GBuffer Albedo", "GBuffer Normal", "Depth", "Shadow Map" };

            for (int i = 0; i < (int)IM_ARRAYSIZE(entries); ++i) {
                if (ImGui::Selectable(entries[i], selected == i)) {
                    selected = i;
                    // bind buffer to the image
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

}