#include <editor/editor_application.h>
#include <core/io/embedded_resource.h>
#include <core/log/error_macros.h>
#include <imgui.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

Error EditorApplication::on_init()
{
    using enum Error;
    Error err;

    err = window_driver.window_set_icon(window, EmbeddedResource::load_icon(L"BALLISTIC_ICON"));
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = window_driver.window_set_titlebar_color(window, RGB(20, 20, 25));
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    ImGuiIO& io = ImGui::GetIO();
    {
        EmbeddedResource::Blob jb = EmbeddedResource::load(L"FONTS_JETBRAINS_MONO_REGULAR_TTF");
        ImFontConfig jb_cfg;
        jb_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void*)jb.data, (int)jb.size, 14.0f, &jb_cfg);

        EmbeddedResource::Blob fa = EmbeddedResource::load(L"FONTS_FA_SOLID_900_OTF");
        static const ImWchar fa_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig fa_cfg;
        fa_cfg.MergeMode = true;
        fa_cfg.PixelSnapH = true;
        fa_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void*)fa.data, (int)fa.size, 14.0f, &fa_cfg, fa_ranges);
        io.Fonts->Build();
    }

    settings.load();
    settings.theme.apply();

    close_project();
    return Ok;
}

void EditorApplication::on_update(float p_dt)
{
    if (mode == Mode::ProjectManager) {
        project_manager.on_update();
        if (project_manager.open_requested) {
            project_manager.open_requested = false;
            open_project(project_manager.open_path);
        }
        return;
    }

    if (!editor_created) {
        if (pending_render_path) return;

        EditorContext ctx{};
        ctx.renderer = &renderer;
        ctx.imgui = &imgui;
        ctx.dev = &dev_tools;
        ctx.render_path = static_cast<EditorRenderPath*>(render_path);
        ctx.project = &project;
        ctx.settings = &settings;

        if (editor.create(ctx) != Error::Ok) return;
        editor_created = true;
    }

    editor.on_update(p_dt);
    if (editor.close_project_requested) {
        editor.close_project_requested = false;
        close_project();
    }
}

void EditorApplication::on_shutdown()
{
    if (editor_created) editor.destroy();
    settings.save();
    project_manager.save_recent();
}

void EditorApplication::open_project(const std::filesystem::path& p_root)
{
    if (project.load(p_root) != Error::Ok) return;

    project_manager.add_recent(p_root, project.name);
    render_path_request(new EditorRenderPath());
    mode = Mode::Editor;

    window_driver.window_set_title(window, project.name + std::string(" - Ballistic Editor"));
}

void EditorApplication::close_project()
{
    if (editor_created) {
        editor.destroy();
        editor_created = false;
    }

    project.unload();
    render_path_request(new ProjectManagerRenderPath());
    mode = Mode::ProjectManager;

    project_manager.selected = -1;
    project_manager.load_recent();

    window_driver.window_set_title(window, "Ballistic Editor - Project Manager");
}

}