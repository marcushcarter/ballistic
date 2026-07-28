
#pragma once
#include <core/application/application.h>
#include <editor/editor.h>
#include <editor/project_manager/project_manager.h>
#include <core/rendering/render_path/editor_render_path.h>
#include <core/rendering/render_path/project_manager_render_path.h>
#include <editor/editor_settings.h>
#include <vector>

namespace ballistic {

struct EditorApplication : Application
{
    enum class Mode { ProjectManager, Editor };
    Mode mode = Mode::ProjectManager;

    std::vector<std::string> titlebar_tabs { "Level", "Profiling" };
    int titlebar_active_tab = 0;

    ProjectManager project_manager;
    Editor editor;
    bool editor_created = false;

    EditorSettings settings;

    drivers::DeviceDriverVulkan::Image logo_image;

    Error on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;

    void open_project(const std::filesystem::path& p_root);
    void close_project();

    void _draw_titlebar();

    bool wants_docking() const override { return true; }
    bool wants_custom_titlebar() const override { return true; }
    
    RenderPath* EditorApplication::create_render_path() { return new EditorRenderPath(); }
};

}