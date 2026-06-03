#include "render_graph_panel.h"
#include "editor/context.h"
#include "graphics/renderer.h"
#include "graphics/render_graph/render_graph.h"

static const char* ImageLayoutShortName(VkImageLayout layout)
{
    switch (layout)
    {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         return "ColorAttachment";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "DepthAttachment";
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         return "ShaderReadOnly";
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:             return "TransferSrc";
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             return "TransferDst";
        case VK_IMAGE_LAYOUT_GENERAL:                          return "General";
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                  return "Present";
        case VK_IMAGE_LAYOUT_UNDEFINED:                        return "Undefined";
        default:                                               return "Other";
    }
}

static bool IsImageResource(const RenderGraph& graph, uint32_t resIdx)
{
    const RenderGraph::Kind k = graph.resources[resIdx].kind;
    return k == RenderGraph::Kind::TransientImage || k == RenderGraph::Kind::ExternalImage;
}

static ImU32 ResourceBarColor(RenderGraph::Kind kind)
{
    switch (kind)
    {
        case RenderGraph::Kind::ExternalImage:  return IM_COL32(100, 160, 220, 180);
        case RenderGraph::Kind::TransientImage: return IM_COL32( 80, 200, 140, 180);
        case RenderGraph::Kind::ExternalBuffer: return IM_COL32(220, 160, 80,  180);
        case RenderGraph::Kind::TransientBuffer:return IM_COL32(200, 120, 80,  180);
        default:                                return IM_COL32(140, 140, 140, 180);
    }
}

void RenderGraphPanel::Draw(EditorContext& ctx)
{
    RenderGraph& graph = ctx.renderer.graph;

    if (ImGui::Begin("Render Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {    
        const int passCount  = (int)graph.plan.size();
        const int resCount   = (int)graph.resources.size();

        if (passCount == 0)
        {
            ImGui::TextDisabled("No passes compiled.");
            ImGui::End();
            return;
        }

        const float panelWidth  = ImGui::GetContentRegionAvail().x;
        const float colW        = panelWidth / (float)passCount;
        const float passRowH    = 28.0f;
        const float resourceRowH= 20.0f;
        const float resourceBarH= 14.0f;
        const float padding     = 2.0f;

        // ── Pass buttons ────────────────────────────────────────────────

        for (int i = 0; i < passCount; ++i)
        {
            const RenderGraph::Step& step = graph.plan[i];
            const RenderGraph::Node* node = step.node;
            if (!node) continue;

            if (i > 0) ImGui::SameLine(0.0f, 0.0f);

            const bool culled = !node->active;
            if (culled) ImGui::BeginDisabled();

            ImGui::PushID(i);
            char label[64];
            snprintf(label, sizeof(label), "%s##pb%d", node->name, i);
            ImGui::Button(node->name, ImVec2(colW - padding, passRowH));
            ImGui::PopID();

            if (culled)
            {
                ImGui::EndDisabled();
                continue;
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::BeginTooltip();
                ImGui::TextDisabled("Barriers: %d", (int)step.barriers.size());
                ImGui::Separator();

                const float colWidth = 240.0f;
                ImGui::Columns(2, nullptr, false);
                ImGui::SetColumnWidth(0, colWidth);
                ImGui::SetColumnWidth(1, colWidth);

                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Reads");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Writes");
                ImGui::NextColumn();
                ImGui::Separator();

                for (const RenderGraph::Use& u : node->uses)
                {
                    if (u.isWrite) continue;
                    if (u.handle.resource >= (uint32_t)resCount) continue;
                    const char* resName = graph.resources[u.handle.resource].name;
                    if (IsImageResource(graph, u.handle.resource))
                    {
                        ImGui::Text("%s", resName);
                        ImGui::SameLine();
                        ImGui::TextDisabled("[%s]", ImageLayoutShortName(u.layout));
                    }
                    else
                    {
                        ImGui::Text("%s", resName);
                        ImGui::SameLine();
                        ImGui::TextDisabled("[buffer]");
                    }
                }

                ImGui::NextColumn();

                for (const RenderGraph::Use& u : node->uses)
                {
                    if (!u.isWrite) continue;
                    if (u.handle.resource >= (uint32_t)resCount) continue;
                    const char* resName = graph.resources[u.handle.resource].name;
                    if (IsImageResource(graph, u.handle.resource))
                    {
                        ImGui::Text("%s", resName);
                        ImGui::SameLine();
                        ImGui::TextDisabled("[%s]", ImageLayoutShortName(u.layout));
                    }
                    else
                    {
                        ImGui::Text("%s", resName);
                        ImGui::SameLine();
                        ImGui::TextDisabled("[buffer]");
                    }
                }

                ImGui::Columns(1);
                ImGui::EndTooltip();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ── Resource lifetime timeline ───────────────────────────────────

        ImGui::BeginChild("##rg_timeline", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* dl       = ImGui::GetWindowDrawList();
        const ImVec2 origin  = ImGui::GetCursorScreenPos();
        float cursorY        = origin.y;

        for (int ri = 0; ri < resCount; ++ri)
        {
            const RenderGraph::Resource& res = graph.resources[ri];

            if (!res.referenced) continue;
            if (res.firstPass < 0 || res.lastPass < 0) continue;
            if (res.firstPass > res.lastPass) continue;
            if (!res.name) continue;

            const float x0 = origin.x + (float)res.firstPass * colW;
            const float x1 = origin.x + ((float)res.lastPass + 1.0f) * colW - padding;
            const float y0 = cursorY  + (resourceRowH - resourceBarH) * 0.5f;
            const float y1 = y0 + resourceBarH;

            const ImU32 barCol  = ResourceBarColor(res.kind);
            const ImU32 textCol = IM_COL32(220, 220, 220, 255);

            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), barCol, 3.0f);
            dl->AddRect      (ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(255,255,255,40), 3.0f);

            // clip name inside bar
            dl->PushClipRect(ImVec2(x0 + 4.0f, y0), ImVec2(x1 - 4.0f, y1), true);
            dl->AddText(ImVec2(x0 + 4.0f, y0 + (resourceBarH - ImGui::GetTextLineHeight()) * 0.5f), textCol, res.name);
            dl->PopClipRect();

            // invisible widget so ImGui knows the row exists for scrolling/layout
            ImGui::SetCursorScreenPos(ImVec2(origin.x, cursorY));
            ImGui::Dummy(ImVec2(panelWidth, resourceRowH));

            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("%s", res.name);
                ImGui::Separator();
                const char* kindStr =
                    res.kind == RenderGraph::Kind::ExternalImage  ? "External Image"  :
                    res.kind == RenderGraph::Kind::TransientImage ? "Transient Image" :
                    res.kind == RenderGraph::Kind::ExternalBuffer ? "External Buffer" : "Transient Buffer";
                ImGui::TextDisabled("Kind:       %s", kindStr);
                ImGui::TextDisabled("First pass: %d", res.firstPass);
                ImGui::TextDisabled("Last pass:  %d", res.lastPass);
                ImGui::TextDisabled("Lifetime:   %d passes", res.lastPass - res.firstPass + 1);
                if (res.kind == RenderGraph::Kind::TransientImage && res.resolvedExtent.width > 0)
                    ImGui::TextDisabled("Extent:     %ux%u", res.resolvedExtent.width, res.resolvedExtent.height);
                ImGui::EndTooltip();
            }

            cursorY += resourceRowH;
        }

        ImGui::EndChild();
    }   
    ImGui::End();
}
