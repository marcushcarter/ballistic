#pragma once
#include <editor/editor_context.h>
#include <imgui.h>

namespace ballistic {

struct Popup
{
    bool open = false;

    virtual ~Popup() = default;
    virtual const char* name() const = 0;
    virtual ImVec2 initial_size() const { return ImVec2(500, 250); }
    virtual void on_open(EditorContext& ctx) { (void)ctx; }
    virtual void draw_contents(EditorContext& ctx) = 0;

    virtual void draw_footer(EditorContext& ctx) {
        (void)ctx;
        const char* labels[] = { "Close" };
        switch (footer_buttons(labels, 1)) {
            case 0: close(); break;
        }
    }

    void close() { open = false; ImGui::CloseCurrentPopup(); }

    static int footer_buttons(const char* const* labels, int count, float bw = 120.0f, unsigned disabled_mask = 0)
    {
        ImGuiStyle& s = ImGui::GetStyle();
        float total = count * bw + s.ItemSpacing.x * (count - 1);
        float avail = ImGui::GetContentRegionAvail().x;
        if (total < avail) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - total) * 0.5f);
        int clicked = -1;
        for (int i = 0; i < count; ++i) {
            if (i) ImGui::SameLine();
            ImGui::PushID(i);
            bool dis = (disabled_mask >> i) & 1u;
            if (dis) ImGui::BeginDisabled();
            if (ImGui::Button(labels[i], ImVec2(bw, 0))) clicked = i;
            if (dis) ImGui::EndDisabled();
            ImGui::PopID();
        }
        return clicked;
    }

    void draw(EditorContext& ctx)
    {
        if (open) {
            ImGui::OpenPopup(name());
            on_open(ctx);
            open = false;
        }

        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(initial_size(), ImGuiCond_Appearing);

        if (!ImGui::BeginPopupModal(name(), nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) return;

        const float footer_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;

        ImGui::BeginChild("##popup_body", ImVec2(0, -footer_h));
        draw_contents(ctx);
        ImGui::EndChild();
        
        draw_footer(ctx);
        
        ImGui::EndPopup();
    }
};

}