#pragma once
#include <core/dev_tools/dev_context.h>
#include <imgui.h>

namespace ballistic {

struct DevPanel
{
    bool open = true;

    virtual ~DevPanel() = default;
    virtual const char* name() const = 0;

    void draw(DevContext& ctx) {
        if (!open) return;

        ImGuiWindowFlags flags = window_flags();
        int style_count = push_style();
        before_begin();
        bool visible = ImGui::Begin(name(), &open, flags);
        if (style_count) ImGui::PopStyleVar(style_count);

        if (visible)
            draw_contents(ctx);

        ImGui::End();
    }

    virtual void draw_contents(DevContext& ctx) = 0;
    virtual ImGuiWindowFlags window_flags() const { return 0; }
    virtual int push_style() { return 0; }
    virtual void before_begin() {}

    virtual bool show_in_editor() const { return true; }
};

}