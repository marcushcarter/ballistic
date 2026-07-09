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

    EditorContext ctx{};
    ctx.renderer = &renderer;
    ctx.dev_tools = &dev_tools;

    err = editor.create(ctx);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

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

    return Ok;
}

void EditorApplication::on_update(float p_dt)
{
    editor.update(p_dt);
    editor.draw();
}

void EditorApplication::on_shutdown()
{
    editor.destroy();
}

}