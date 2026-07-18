
#pragma once
#include <core/application/application.h>
#include <editor/editor.h>
#include <editor/project_manager/project_manager.h>
#include <core/rendering/render_path/editor_render_path.h>
#include <core/rendering/render_path/project_manager_render_path.h>
#include <editor/editor_settings.h>

namespace ballistic {

struct EditorApplication : Application
{
    enum class Mode { ProjectManager, Editor };
    Mode mode = Mode::ProjectManager;

    ProjectManager project_manager;
    Editor editor;
    bool editor_created = false;

    EditorSettings settings;

    Error on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;

    bool wants_docking() const override { return true; }
    RenderPath* EditorApplication::create_render_path() { return new EditorRenderPath(); }

    void open_project(const std::filesystem::path& p_root);
    void close_project();
};

}