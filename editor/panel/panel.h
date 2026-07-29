#pragma once
#include <editor/editor_context.h>
#include <imgui.h>

namespace ballistic {

struct Panel
{
    bool open = true;

    virtual ~Panel() = default;
    virtual const char* name() const = 0;

    void draw(EditorContext& ctx) {
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

    virtual void draw_contents(EditorContext& ctx) = 0;
    virtual ImGuiWindowFlags window_flags() const { return 0; }
    virtual int push_style() { return 0; }
    virtual void before_begin() {}
};

}