#include <core/dev_tools/profiler/profiler_timeline.h>
#include <core/rendering/renderer.h>
#include <imgui.h>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace ballistic {

static ImU32 rg_category_u32(const char* cat, float alpha = 1.0f)
{
    ImVec4 c(0.70f, 0.70f, 0.70f, alpha);
    if (cat && cat[0]) {
        uint64_t h = 1469598103934665603ull;
        for (const char* p = cat; *p; ++p) { h ^= (uint8_t)*p; h *= 1099511628211ull; }
        c = (ImVec4)ImColor::HSV((float)(h % 360) / 360.0f, 0.55f, 0.95f);
        c.w = alpha;
    }
    return ImGui::GetColorU32(c);
}

static void property_row(const char* name, const char* fmt, ...)
{
    constexpr float tab_width = 200.0f;
    ImGui::TextUnformatted(name);
    ImGui::SameLine(tab_width);
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void ProfilerTimeline::draw(DevContext& ctx)
{
    RenderGraphProfiler& prof = ctx.renderer->graph.profiler;

    // selected_draw = nullptr;
    // selected_pass = nullptr;

    const std::vector<RenderGraphProfiler::Timing>& src = prof.results;
    const float total_ms = (float)prof.total_ms;
    
    ImVec2 avail = ImGui::GetContentRegionAvail();

    if (src.empty() || total_ms <= 0.0f) {
        const char* text = "No GPU timing yet.";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail.x - textSize.x) * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (avail.y - textSize.y) * 0.5f);
        ImGui::TextDisabled("%s", text);
    } else {
        const size_t n = src.size();
        static std::vector<float> start;
        static std::vector<float> cursor;
        start.assign(n, 0.0f);
        cursor.assign(n, 0.0f);

        float frame_cursor = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            const RenderGraphProfiler::Timing& t = src[i];
            if (t.kind == RenderGraphProfiler::MarkKind::Pass || t.parent == RenderGraphProfiler::INVALID) {
                start[i] = frame_cursor + (float)t.gap_ms;
                frame_cursor = start[i] + (float)t.gpu_ms;
            } else {
                const uint32_t p = t.parent;
                start[i] = cursor[p] + (float)t.gap_ms;
                cursor[p] = start[i] + (float)t.gpu_ms;
            }
            cursor[i] = start[i];
        }

        float canvas_w = avail.x;
        float canvas_h = avail.y;

        static float view_start_ms = 0.0f;
        static float view_end_ms = -1.0f;
        static float scroll_ms = 0.0f;

        static bool selecting = false;
        static float select_start_ms = 0.0f;
        static float select_end_ms = 0.0f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 origin = ImGui::GetCursorScreenPos();
        bool hovered = false;

        float visible_start = 0.0f;
        float visible_end = total_ms;
        float range_ms = total_ms;

        if (view_end_ms > view_start_ms) {
            range_ms = view_end_ms - view_start_ms;
            float max_scroll = total_ms - range_ms;
            scroll_ms = std::clamp(scroll_ms, 0.0f, max_scroll);
            visible_start = scroll_ms;
            visible_end = visible_start + range_ms;
        } else {                
            scroll_ms = 0.0f;
        }

        float px = canvas_w / range_ms;
        float content_w = canvas_w;

        float H = canvas_h;
        ImGui::InvisibleButton("TimelineInput", ImVec2(content_w, H), ImGuiButtonFlags_MouseButtonLeft);
        hovered = ImGui::IsItemHovered();

        dl->PushClipRect(origin, ImVec2(origin.x + canvas_w, origin.y + H), true);

        if (hovered && io.MouseWheelH != 0.0f) {
            if (view_end_ms > view_start_ms) {
                float range = view_end_ms - view_start_ms;
                scroll_ms -= io.MouseWheelH * range * 0.1f;
                float max_scroll = total_ms - range;
                scroll_ms = std::clamp(scroll_ms, 0.0f, max_scroll);
            }
        }

        if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            selecting = false;
            view_start_ms = 0.0f;
            view_end_ms = -1.0f;
            scroll_ms = 0.0f;
        } else if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            selecting = true;
            select_start_ms = (io.MousePos.x - origin.x) / px + visible_start;
            select_end_ms = select_start_ms;
        }

        if (selecting) {
            select_end_ms = (io.MousePos.x - origin.x) / px + visible_start;
            select_start_ms = std::clamp(select_start_ms, 0.0f, total_ms);
            select_end_ms = std::clamp(select_end_ms, 0.0f, total_ms);
        }

        if (selecting && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            selecting = false;
            float a = std::clamp(std::min(select_start_ms, select_end_ms), 0.0f, total_ms);
            float b = std::clamp(std::max(select_start_ms, select_end_ms), 0.0f, total_ms);
            if (b - a > 0.01f) {
                view_start_ms = a;
                view_end_ms = b;
                scroll_ms = a;
            }
        }

        // Grid.
        {
            float visible_ms = canvas_w / px;
            float start_ms = visible_start;

            float block_ms;
            if (visible_ms >= 1.5f) block_ms = 1.0f;
            else if (visible_ms >= 0.15f) block_ms = 0.1f;
            else block_ms = 0.01f;

            int first_index = (int)floorf(start_ms / block_ms);
            float end = visible_end;

            for (int i = first_index; ; i++) {
                float t = i * block_ms;
                if (t > end) break;

                float x0 = origin.x + (t - visible_start) * px;
                float x1 = origin.x + (t + block_ms - visible_start) * px;

                bool dark = (i & 1) == 0;
                dl->AddRectFilled(ImVec2(x0, origin.y), ImVec2(x1, origin.y + H), dark ? IM_COL32(45, 45, 45, 255) : IM_COL32(55, 55, 55, 255));

                char label[32];
                if (block_ms == 1.0f) snprintf(label, sizeof(label), "%.0f ms", t);
                else if (block_ms == 0.1f) snprintf(label, sizeof(label), "%.1f ms", t);
                else snprintf(label, sizeof(label), "%.2f ms", t);

                dl->AddText(ImVec2(x0 + 3.0f, origin.y + H - ImGui::GetTextLineHeight()), IM_COL32(220, 220, 220, 180), label);
            }
        }

        const float gap = 3.0f;
        const float sec_h = 18.0f;
        const float pass_h = 22.0f;
        const float draw_h = 14.0f;

        float y_sec0  = origin.y;
        float y_sec1 = y_sec0 + sec_h;
        float y_pass0 = y_sec1 + gap;
        float y_pass1 = y_pass0 + pass_h;
        float y_draw0 = y_pass1 + gap;
        float y_draw1 = y_draw0 + draw_h;

        auto label_in = [&](ImVec2 a, ImVec2 b, const char* txt, ImU32 col) {
            if (!txt || !txt[0]) return;
            float w = b.x - a.x;
            if (w < 6.0f) return;
            ImVec2 ts = ImGui::CalcTextSize(txt);
            float x = a.x + 4.0f;
            x = std::max(x, origin.x + 4.0f);
            if (x + ts.x > b.x - 4.0f) ts.x = b.x - x - 4.0f;
            dl->PushClipRect(a, b, true);
            ImVec2 p(x, a.y + ((b.y - a.y) - ts.y) * 0.5f);
            dl->AddText(ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 140), txt);
            dl->AddText(p, col, txt);
            dl->PopClipRect();
        };

        auto bar = [&](float ms_start, float ms_len, float y0, float y1, ImU32 fill, bool* r_hovered = nullptr) -> ImVec2 {
            float x0 = origin.x + (ms_start - visible_start) * px;
            float x1 = origin.x + (ms_start + ms_len - visible_start) * px;
            ImVec2 a(x0 + 1.0f, y0);
            ImVec2 b(x1 - 1.0f, y1);
            bool is_hovered = io.MousePos.x >= a.x && io.MousePos.x <= b.x && io.MousePos.y >= a.y && io.MousePos.y <= b.y;
            if (r_hovered) *r_hovered = is_hovered;
            if (b.x > a.x) dl->AddRectFilled(a, b, (r_hovered && is_hovered) ? IM_COL32(255, 50, 50, 255) : fill, 3.0f);
            return ImVec2(a.x, b.x);
        };

        // Categories.
        {
            float x_ms = 0.0f;
            const char* run_cat = nullptr;
            float run_ms = 0.0f;

            auto flush = [&]() {
                if (!run_cat) return;
                ImVec2 x = bar(x_ms, run_ms, y_sec0, y_sec1, rg_category_u32(run_cat));
                label_in(ImVec2(x.x, y_sec0), ImVec2(x.y, y_sec1), run_cat[0] ? run_cat : "(uncat)", IM_COL32_WHITE);
                x_ms += run_ms;
                run_ms = 0.0f;
            };

            for (const RenderGraphProfiler::Timing& t : src) {
                if (t.kind != RenderGraphProfiler::MarkKind::Pass) continue;
                const char* cat = t.category ? t.category : "";
                const float span = (float)(t.gap_ms + t.gpu_ms);
                if (run_cat && std::strcmp(run_cat, cat) == 0) { run_ms += span; continue; }
                flush();
                run_cat = cat;
                run_ms = span;
            }

            flush();
        }

        // Passes.
        for (size_t i = 0; i < n; ++i) {
            const RenderGraphProfiler::Timing& t = src[i];
            if (t.kind != RenderGraphProfiler::MarkKind::Pass) continue;

            bool bar_hovered = false;
            ImVec2 x = bar(start[i], (float)t.gpu_ms, y_pass0, y_pass1, rg_category_u32(t.category), &bar_hovered);
            if (bar_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) selected_pass = (selected_pass == &t) ? nullptr : &t;
            label_in(ImVec2(x.x, y_pass0), ImVec2(x.y, y_pass1), t.name, IM_COL32_WHITE);


            // if (bar_hovered) {
            //     ImGui::BeginTooltip();
            //     ImGui::EndTooltip();
            // }
        }

        // Draw calls.
        for (size_t i = 0; i < n; ++i) {
            const RenderGraphProfiler::Timing& t = src[i];
            if (t.kind != RenderGraphProfiler::MarkKind::Draw) continue;

            const uint32_t p = t.parent;
            if (p == RenderGraphProfiler::INVALID) continue;
            const float parent_end = start[p] + (float)src[p].gpu_ms;
            const float a_ms = start[i];
            if (a_ms >= parent_end) continue;
            const float len_ms = std::min((float)t.gpu_ms, parent_end - a_ms);

            bool bar_hovered = false;
            ImVec2 x = bar(a_ms, len_ms, y_draw0, y_draw1, rg_category_u32(src[p].category, 0.45f), &bar_hovered);
            if (bar_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) selected_draw = (selected_draw == &t) ? nullptr : &t;

            const bool named = t.name && t.name[0];
            char lbl[16];
            if (!named) snprintf(lbl, sizeof(lbl), "%u", t.ordinal);
            label_in(ImVec2(x.x, y_draw0), ImVec2(x.y, y_draw1), named ? t.name : lbl, IM_COL32_WHITE);

            if (bar_hovered) {
                ImGui::BeginTooltip();

                if (named) ImGui::TextUnformatted(t.name);
                else ImGui::Text("Draw %u", t.ordinal);
                property_row("Type", "%s", t.type);
                ImGui::Separator();
                property_row("Time", "%.3f ms", t.gpu_ms);
                property_row("Raw", "%.3f ms", t.raw_ms);
                property_row("Setup", "%.3f ms", t.gap_ms);
                property_row("Pass", "%s", src[p].name);
                property_row("Index", "%u", t.ordinal);
                property_row("Pixel Count", "%d", 0);
                ImGui::Spacing();
                ImGui::Spacing();

                ImGui::Text("Owner Object");
                property_row("Name", "%s", "n/a");
                property_row("Location", "%s", "n/a");
                property_row("Type", "%s", "n/a");
                ImGui::Spacing();
                ImGui::Spacing();

                ImGui::Text("Primitives");
                property_row("Triangles", "%d", 0);
                property_row("Vertices", "%d", 0);
                property_row("Instances", "%d", 0);
                property_row("Total Triangles", "%d", 0);
                ImGui::Spacing();
                ImGui::Spacing();

                ImGui::Text("Shader");
                property_row("Pass", "%s", src[p].category);
                property_row("Name", "%s", "n/a");
                property_row("Location", "%s", "n/a");

                ImGui::EndTooltip();
            }
        }

        // Highlight.
        if (selecting) {
            float a = std::min(select_start_ms, select_end_ms);
            float b = std::max(select_start_ms, select_end_ms);
            dl->AddRectFilled(ImVec2(origin.x + (a - visible_start) * px, origin.y), ImVec2(origin.x + (b - visible_start) * px, origin.y + H), IM_COL32(100, 150, 255, 50));
        }

        // Hover bar.
        if (hovered) {
            dl->AddLine(ImVec2(io.MousePos.x, origin.y), ImVec2(io.MousePos.x, origin.y + H), IM_COL32(255, 255, 255, 120), 1.0f);
        }

        dl->PopClipRect();
    }
}

}