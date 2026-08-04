#include <editor/panel/asset_browser/asset_browser_grid.h>
#include <editor/editor_resources.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/io/path.h>
#include <imgui.h>
#include <cstdio>

namespace ballistic {

bool AssetBrowserGrid::_draw_card(ImTextureID p_texture, const char* p_name, const char* p_type, const std::filesystem::path& p_path)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + card_width, p0.y + card_height);
    const float rounding = 4.0f;
    const float pad = 6.0f;
    const float img_pad = 4.0f;

    ImGui::InvisibleButton("##card", ImVec2(card_width, card_height));

    const bool double_clicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        const std::string path_str = p_path.string();
        ImGui::SetDragDropPayload("ASSET_PATH", path_str.c_str(), path_str.size() + 1);
        ImGui::Text("%s", p_name);
        ImGui::EndDragDropSource();
    }

    const ImVec2 thumb1(p0.x + card_width, p0.y + card_width);
    const ImVec2 foot0(p0.x, thumb1.y);
    const ImVec2 sh_off(-2.0f, 2.0f);
    const ImVec2 img0(p0.x + img_pad, p0.y + img_pad);
    const ImVec2 img1(thumb1.x - img_pad, thumb1.y - img_pad);
    
    dl->AddRectFilled(ImVec2(p0.x + sh_off.x, p0.y + sh_off.y), ImVec2(p1.x + sh_off.x, p1.y + sh_off.y), IM_COL32(0, 0, 0, 160), rounding, ImDrawFlags_RoundCornersBottom);
    dl->AddRectFilled(p0, thumb1, IM_COL32(30, 30, 34, 255));
    if (p_texture) dl->AddImage(p_texture, img0, img1, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
    dl->AddRectFilled(foot0, p1, IM_COL32(20, 20, 24, 255), rounding, ImDrawFlags_RoundCornersBottom);

    if (ImGui::IsItemHovered()) dl->AddRect(p0, p1, IM_COL32(255, 255, 255, 255), rounding, ImDrawFlags_RoundCornersBottom, 1.0f);

    const ImVec2 name_min(foot0.x + pad, foot0.y + pad);
    const ImVec2 name_max(p1.x - pad, name_min.y + ImGui::GetTextLineHeight());
    dl->PushClipRect(name_min, name_max, true);
    dl->AddText(name_min, IM_COL32(224, 224, 230, 255), p_name);
    dl->PopClipRect();

    if (p_type && *p_type) {
        const ImVec2 ts = ImGui::CalcTextSize(p_type);
        const ImVec2 type_pos(p1.x - pad - ts.x, p1.y - pad - ts.y);
        dl->AddText(type_pos, IM_COL32(120, 120, 128, 255), p_type);
    }

    return double_clicked;
}

void AssetBrowserGrid::draw(EditorContext& ctx, std::filesystem::path& selected, const char* search_buf)
{
    const float min_gap = 16.0f;
    const float row_gap = 8.0f;
    const float avail = ImGui::GetContentRegionAvail().x;

    int columns = (int)((avail + min_gap) / (card_width + min_gap));
    if (columns < 1) columns = 1;

    float gap = min_gap;
    if (columns > 1) gap = (avail - (float)columns * (float)card_width) / (float)(columns - 1);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(min_gap, row_gap));

    if (!selected.empty() && std::filesystem::exists(selected)) {
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : std::filesystem::directory_iterator(selected)) entries.push_back(entry);
        std::sort(entries.begin(), entries.end(),
            [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
                const bool a_dir = a.is_directory();
                const bool b_dir = b.is_directory();
                if (a_dir != b_dir) return a_dir;
                const std::string an = a.path().filename().string();
                const std::string bn = b.path().filename().string();
                return std::lexicographical_compare(an.begin(), an.end(), bn.begin(), bn.end(), [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });
            });

        std::string query = search_buf;
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        int i = 0;
        for (const auto& entry : entries) {
            // if (entry.is_directory()) continue;

            if (!query.empty()) {
                std::string hay = entry.path().filename().string();
                std::transform(hay.begin(), hay.end(), hay.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });
                if (hay.find(query) == std::string::npos) continue;
            }

            const std::string name = entry.path().stem().string();
            std::string type = entry.path().extension().string();
            for (char& c : type) c = (char)toupper((unsigned char)c);

            if (i % columns != 0) ImGui::SameLine(0.0f, gap);
            
            ImGui::PushID(i);
            VkDescriptorSet logo_set = ctx.imgui->texture_cache.get(ctx.resources->test_thumbnail.image_view);
            if (entry.is_directory()) logo_set = VK_NULL_HANDLE;
            const bool activated = _draw_card((ImTextureID)logo_set, name.c_str(), type.c_str(), entry.path());

            if (entry.is_directory() && ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                    Paths::asset_move((const char*)payload->Data, entry.path());
                }
                ImGui::EndDragDropTarget();
            }

            if (activated && entry.is_directory()) selected = entry.path();
            ImGui::PopID();

            i++;
        }
    }

    ImGui::PopStyleVar();
    
    if (ImGui::BeginPopupContextWindow("##grid_ctx", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        ImGui::MenuItem("New Folder", nullptr, false, /*enabled=*/false);
        ImGui::MenuItem("Import...",  nullptr, false, /*enabled=*/false);
        ImGui::EndPopup();
    }
    
}

}