
#pragma once
#include <core/application/application.h>
#include <editor/editor.h>
#include <core/rendering/render_path/editor_render_path.h>

namespace ballistic {

struct EditorApplication : Application
{
    Editor editor;

    Error on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;

    bool wants_docking() const override { return true; }
    RenderPath* EditorApplication::create_render_path() { return new EditorRenderPath(); }
};

}