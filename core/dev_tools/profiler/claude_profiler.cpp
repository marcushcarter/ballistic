#include <core/dev_tools/profiler/claude_profiler.h>
#include <core/rendering/renderer.h>
#include <drivers/imgui/imgui_driver.h>
#include <drivers/windows/dialogs_win32.h>
#include <core/log/log.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>
#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include <string>
#include <fstream>

namespace ballistic {

static const char* rg_layout_str(VkImageLayout l)
{
    switch (l) {
        case VK_IMAGE_LAYOUT_UNDEFINED:                        return "UNDEFINED";
        case VK_IMAGE_LAYOUT_GENERAL:                          return "GENERAL";
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         return "COLOR_ATTACH";
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:         return "DEPTH_ATTACH";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "DS_ATTACH";
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:          return "DEPTH_RO";
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         return "SHADER_RO";
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:             return "XFER_SRC";
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             return "XFER_DST";
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                  return "PRESENT";
        default:                                               return "?";
    }
}

static std::string rg_format_str(VkFormat f)
{
    switch (f) {
        case VK_FORMAT_R8G8B8A8_UNORM:       return "RGBA8_UNORM";
        case VK_FORMAT_B8G8R8A8_UNORM:       return "BGRA8_UNORM";
        case VK_FORMAT_B8G8R8A8_SRGB:        return "BGRA8_SRGB";
        case VK_FORMAT_R16G16B16A16_SFLOAT:  return "RGBA16F";
        case VK_FORMAT_R32G32B32A32_SFLOAT:  return "RGBA32F";
        case VK_FORMAT_D32_SFLOAT:           return "D32F";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:   return "D32F_S8";
        case VK_FORMAT_D24_UNORM_S8_UINT:    return "D24_S8";
        case VK_FORMAT_UNDEFINED:            return "-";
        default:                             return vk::to_string(vk::Format(f));
    }
}

static const char* rg_loadop_str(VkAttachmentLoadOp op)
{
    switch (op) {
        case VK_ATTACHMENT_LOAD_OP_LOAD:      return "LOAD";
        case VK_ATTACHMENT_LOAD_OP_CLEAR:     return "CLEAR";
        case VK_ATTACHMENT_LOAD_OP_DONT_CARE: return "DONT_CARE";
        default:                              return "?";
    }
}

static std::string rg_stage_str(VkPipelineStageFlags2 s) { return vk::to_string(vk::PipelineStageFlags2(s)); }
static std::string rg_access_str(VkAccessFlags2 a)       { return vk::to_string(vk::AccessFlags2(a)); }

static const char* rg_res_name(RenderGraph& g, uint64_t id)
{
    auto it = g.debug_names.find(id);
    return it != g.debug_names.end() ? it->second.c_str() : "?";
}

void ClaudeProfiler::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(750, 400), ImGuiCond_FirstUseEver);
}

void ClaudeProfiler::draw_contents(DevContext& ctx)
{
    
    if (!ctx.renderer) { ImGui::TextUnformatted("No renderer."); return; }
    RenderGraph& g = ctx.renderer->graph;

    // Timings are keyed by name_id; match to nodes by interning the pass name.
    std::unordered_map<uint64_t, double> ms_by_id;
    ms_by_id.reserve(g.profiler.last_results.size());
    for (const RenderGraph::PassTiming& t : g.profiler.last_results)
        ms_by_id[t.name_id] = t.gpu_ms;

    uint32_t active = 0;
    for (const RenderGraph::Node& n : g.nodes) if (!n.culled) ++active;

    // ---- summary ----
    ImGui::Text("Frame %llu   |   Passes: %u active / %zu total   |   Images: %zu   Buffers: %zu",
        (unsigned long long)ctx.renderer->frame_number, active, g.nodes.size(),
        g.image_resources.size(), g.buffer_resources.size());
    if (g.profiler.enabled)
        ImGui::Text("GPU total: %.3f ms", g.profiler.last_total_ms);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.2f, 1.0f), "GPU timing disabled (timestampValidBits == 0).");
    ImGui::TextDisabled("Structure = last built frame; GPU timings lag by frames-in-flight.");
    ImGui::Separator();

    if (g.nodes.empty()) { ImGui::TextUnformatted("Graph not built yet."); return; }

    if (!ImGui::BeginTabBar("rg_tabs")) return;

    // =========================================================
    //  TIMINGS
    // =========================================================
    if (ImGui::BeginTabItem("Timings")) {
        if (!g.profiler.enabled) {
            ImGui::TextUnformatted("Disabled.");
        } else if (g.profiler.last_results.empty()) {
            ImGui::TextUnformatted("Warming up...");
        } else {
            double max_ms = 0.0;
            for (const RenderGraph::PassTiming& t : g.profiler.last_results) max_ms = std::max(max_ms, t.gpu_ms);
            if (max_ms <= 0.0) max_ms = 1.0;

            if (ImGui::BeginTable("timings", 3,
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 0.35f);
                ImGui::TableSetupColumn("ms",   ImGuiTableColumnFlags_WidthFixed,   70.0f);
                ImGui::TableSetupColumn("",     ImGuiTableColumnFlags_WidthStretch, 0.55f);
                ImGui::TableHeadersRow();

                for (const RenderGraph::PassTiming& t : g.profiler.last_results) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(t.name);
                    ImGui::TableNextColumn(); ImGui::Text("%.3f", t.gpu_ms);
                    ImGui::TableNextColumn();
                    ImGui::ProgressBar((float)(t.gpu_ms / max_ms), ImVec2(-1, 0), "");
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndTabItem();
    }

    // =========================================================
    //  PASSES
    // =========================================================
    if (ImGui::BeginTabItem("Passes")) {
        static char filter[64] = {};
        static bool hide_culled = false;
        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##filter", "filter passes", filter, sizeof(filter));
        ImGui::SameLine();
        ImGui::Checkbox("hide culled", &hide_culled);
        ImGui::Separator();

        const ImVec4 col_read (0.55f, 0.85f, 0.55f, 1.0f);
        const ImVec4 col_write(0.90f, 0.55f, 0.55f, 1.0f);
        const ImVec4 col_att  (0.90f, 0.85f, 0.45f, 1.0f);
        const ImVec4 col_dim  (0.55f, 0.55f, 0.55f, 1.0f);

        for (int ni = 0; ni < (int)g.nodes.size(); ++ni) {
            RenderGraph::Node& node = g.nodes[ni];
            const char* pname = node.pass ? node.pass->name.c_str() : "<null>";

            if (hide_culled && node.culled) continue;
            if (filter[0] && !strstr(pname, filter)) continue;

            ImGui::PushID(ni);

            double pms = -1.0;
            if (node.pass) {
                auto it = ms_by_id.find(RenderGraph::intern(node.pass->name));
                if (it != ms_by_id.end()) pms = it->second;
            }

            if (ni != 0) ImGui::Separator();

            if (node.culled)
                ImGui::TextColored(col_dim, "[%d] %s  (culled)", ni, pname);
            else if (pms >= 0.0)
                ImGui::Text("[%d] %s  -  %.3f ms", ni, pname, pms);
            else
                ImGui::Text("[%d] %s", ni, pname);

            {
                ImGui::Indent();
                if (node.has_render_pass)
                    ImGui::TextDisabled("render pass  |  area %ux%u  |  barriers img=%zu buf=%zu",
                        node.area.width, node.area.height,
                        node.pre_image_barriers.size(), node.pre_buffer_barriers.size());
                else
                    ImGui::TextDisabled("compute/transfer  |  barriers img=%zu buf=%zu",
                        node.pre_image_barriers.size(), node.pre_buffer_barriers.size());

                // image accesses, grouped
                for (const RenderGraph::ImageAccess& a : node.image_accesses) {
                    const char* rn = rg_res_name(g, a.name_id);
                    const char* kind = "?";
                    if (a.resource_index >= 0)
                        kind = (g.image_resources[a.resource_index].kind == RenderGraph::ResourceKind::Imported) ? "imp" : "tra";

                    if (a.is_attachment) {
                        ImGui::TextColored(col_att, "  [att%s] %s (%s)  %s  load=%s",
                            a.is_depth ? "/depth" : "", rn, kind, rg_layout_str(a.layout), rg_loadop_str(a.load_op));
                    } else if (a.is_write) {
                        ImGui::TextColored(col_write, "  [w img] %s (%s)  %s", rn, kind, rg_layout_str(a.layout));
                    } else {
                        ImGui::TextColored(col_read, "  [r img] %s (%s)  %s", rn, kind, rg_layout_str(a.layout));
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("stage: %s\naccess: %s",
                            rg_stage_str(a.stage).c_str(), rg_access_str(a.access).c_str());
                }

                // buffer accesses
                for (const RenderGraph::BufferAccess& a : node.buffer_accesses) {
                    const char* rn = rg_res_name(g, a.name_id);
                    ImGui::TextColored(a.is_write ? col_write : col_read,
                        "  [%s buf] %s", a.is_write ? "w" : "r", rn);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("stage: %s\naccess: %s",
                            rg_stage_str(a.stage).c_str(), rg_access_str(a.access).c_str());
                }

                // barrier detail
                if (!node.pre_image_barriers.empty() && ImGui::TreeNode("pre image barriers")) {
                    for (const RenderGraph::ImageBarrier& b : node.pre_image_barriers) {
                        ImGui::BulletText("%s -> %s", rg_layout_str(b.old_layout), rg_layout_str(b.new_layout));
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("src: %s / %s\ndst: %s / %s",
                                rg_stage_str(b.src_stage).c_str(), rg_access_str(b.src_access).c_str(),
                                rg_stage_str(b.dst_stage).c_str(), rg_access_str(b.dst_access).c_str());
                    }
                    ImGui::TreePop();
                }
                if (!node.pre_buffer_barriers.empty() && ImGui::TreeNode("pre buffer barriers")) {
                    for (const RenderGraph::BufferBarrier& b : node.pre_buffer_barriers) {
                        ImGui::BulletText("%s / %s  ->  %s / %s",
                            rg_stage_str(b.src_stage).c_str(), rg_access_str(b.src_access).c_str(),
                            rg_stage_str(b.dst_stage).c_str(), rg_access_str(b.dst_access).c_str());
                    }
                    ImGui::TreePop();
                }

                ImGui::Unindent();
            }
            ImGui::PopID();
        }
        ImGui::EndTabItem();
    }

    // =========================================================
    //  RESOURCES
    // =========================================================
    if (ImGui::BeginTabItem("Resources")) {
        auto producer_name = [&](int prod) -> const char* {
            if (prod >= 0 && prod < (int)g.nodes.size() && g.nodes[prod].pass) return g.nodes[prod].pass->name.c_str();
            return "-";
        };

        if (ImGui::CollapsingHeader("Images", ImGuiTreeNodeFlags_DefaultOpen) &&
            ImGui::BeginTable("imgres", 7,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Kind");
            ImGui::TableSetupColumn("Format");
            ImGui::TableSetupColumn("Life");
            ImGui::TableSetupColumn("Producer");
            ImGui::TableSetupColumn("R");
            ImGui::TableSetupColumn("W");
            ImGui::TableHeadersRow();

            for (const RenderGraph::ImageResource& r : g.image_resources) {
                const bool imported = (r.kind == RenderGraph::ResourceKind::Imported);
                VkFormat fmt = (imported && r.image) ? r.image->format : r.image_create_info.format;

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(rg_res_name(g, r.name_id));
                ImGui::TableNextColumn(); ImGui::TextUnformatted(imported ? "imported" : "transient");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(rg_format_str(fmt).c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d..%d", r.first_use, r.last_use);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(producer_name(r.producer));
                ImGui::TableNextColumn(); ImGui::TextUnformatted(r.read ? "R" : "-");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(r.written ? "W" : "-");
            }
            ImGui::EndTable();
        }

        if (ImGui::CollapsingHeader("Buffers", ImGuiTreeNodeFlags_DefaultOpen) &&
            ImGui::BeginTable("bufres", 7,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Kind");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Mem");
            ImGui::TableSetupColumn("Life");
            ImGui::TableSetupColumn("Producer");
            ImGui::TableSetupColumn("R/W");
            ImGui::TableHeadersRow();

            for (const RenderGraph::BufferResource& r : g.buffer_resources) {
                const bool imported = (r.kind == RenderGraph::ResourceKind::Imported);
                VkDeviceSize sz = (imported && r.buffer) ? r.buffer->size : r.buffer_create_info.size;
                bool host = r.buffer_create_info.memory == drivers::DeviceDriverVulkan::BufferCreateInfo::Memory::HostVisible;

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(rg_res_name(g, r.name_id));
                ImGui::TableNextColumn(); ImGui::TextUnformatted(imported ? "imported" : "transient");
                ImGui::TableNextColumn(); ImGui::Text("%llu B", (unsigned long long)sz);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(host ? "host" : "device");
                ImGui::TableNextColumn(); ImGui::Text("%d..%d", r.first_use, r.last_use);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(producer_name(r.producer));
                ImGui::TableNextColumn(); ImGui::Text("%s%s", r.read ? "R" : "-", r.written ? "W" : "-");
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

}