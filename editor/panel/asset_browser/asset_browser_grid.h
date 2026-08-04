#pragma once
#include <editor/editor_context.h>
#include <core/rendering/render_graph_profiler.h>
#include <imgui.h>
#include <filesystem>

namespace ballistic {

struct AssetBrowserGrid
{
    float card_width = 150.0f;
    float card_height = 220.0f;

    bool _draw_card(ImTextureID p_texture, const char* p_name, const char* p_type, const std::filesystem::path& p_path, float p_progress = 0.5);
    void draw(EditorContext& ctx, std::filesystem::path& selected, const char* search_buf);
};

}