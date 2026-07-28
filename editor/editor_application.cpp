#include <editor/editor_application.h>
#include <core/io/embedded_resource.h>
#include <core/io/image_io.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

Error EditorApplication::on_init()
{
    using enum Error;

    ImageData logo = ImageIO::load_from_resource(L"LOGOS_ICON_PNG");
    if (logo.pixels && logo.width > 0 && logo.height > 0) {
        const uint32_t w = (uint32_t)logo.width;
        const uint32_t h = (uint32_t)logo.height;

        const unsigned char* rgba = logo.pixels;
        std::vector<unsigned char> expanded;
        if (logo.channels != 4) {
            expanded.resize((size_t)w * h * 4);
            const int c = logo.channels;
            for (size_t i = 0; i < (size_t)w * h; ++i) {
                expanded[i*4+0] = logo.pixels[i*c+0];
                expanded[i*4+1] = c > 1 ? logo.pixels[i*c+1] : logo.pixels[i*c+0];
                expanded[i*4+2] = c > 2 ? logo.pixels[i*c+2] : logo.pixels[i*c+0];
                expanded[i*4+3] = c > 3 ? logo.pixels[i*c+3] : 255;
            }
            rgba = expanded.data();
        }

        logo_image = dd.image_create_texture(rgba, w, h, "editor_logo");
    }
    ImageIO::free_image(logo);

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
    _draw_titlebar();

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
        ctx.render_path = static_cast<EditorRenderPath*>(render_path);
        ctx.project = &project;
        ctx.settings = &settings;

        if (editor.create(ctx) != Error::Ok) return;
        editor_created = true;
    }

    editor.active_tab = titlebar_active_tab;

    editor.on_update(p_dt);
    if (editor.close_project_requested) {
        editor.close_project_requested = false;
        close_project();
    }
}

void EditorApplication::on_shutdown()
{
    dd.device_wait_idle();
    dd.image_free(logo_image);

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

void EditorApplication::_draw_titlebar()
{
    const float TAB_H = 24.0f;
    const float BTN_W = 46.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 7));

    const float MENU_H = ImGui::GetFrameHeight();
    const float H = MENU_H + TAB_H;
    const float LOGO = H;
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    if (!ImGui::BeginViewportSideBar("##BallisticTitlebar", ImGui::GetMainViewport(), ImGuiDir_Up, H, flags)) {
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }

    const ImVec2 origin = ImGui::GetWindowPos();
    const float width = ImGui::GetWindowWidth();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    window_driver.window_titlebar_reset(window, (int)H);
    auto blocker = [&](ImVec2 mn, ImVec2 mx) {
        window_driver.window_titlebar_add_rect(window, (long)(mn.x - origin.x), (long)(mn.y - origin.y), (long)(mx.x - origin.x), (long)(mx.y - origin.y));
    };

    if (ImGui::BeginMenuBar()) {
        
        ImGui::SetCursorPosX(LOGO + 6.0f);
        float menu_x0 = ImGui::GetCursorScreenPos().x;
        if (editor_created && mode == Mode::Editor) editor.draw_menu();
        float menu_x1 = ImGui::GetCursorScreenPos().x;
        if (menu_x1 > menu_x0) blocker(ImVec2(menu_x0, origin.y), ImVec2(menu_x1, origin.y + MENU_H));

        float btns_x = origin.x + width - BTN_W * 3.0f;
        
        const std::string& title = project.name.empty() ? std::string("Ballistic Editor") : project.name;
        ImVec2 ts = ImGui::CalcTextSize(title.c_str());
        dl->AddText(ImVec2(btns_x - ts.x - 16.0f, origin.y + (MENU_H - ts.y) * 0.5f), IM_COL32(200, 200, 205, 255), title.c_str());

        ImGui::SetCursorScreenPos(ImVec2(btns_x, origin.y));
        auto ctrl = [&](const char* id, int glyph, bool danger) -> bool {
            ImVec2 p = ImGui::GetCursorScreenPos();
            bool pressed = ImGui::InvisibleButton(id, ImVec2(BTN_W, MENU_H));
            blocker(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            if (ImGui::IsItemHovered()) dl->AddRectFilled(p, ImVec2(p.x + BTN_W, p.y + MENU_H), danger ? IM_COL32(196, 43, 28, 255) : IM_COL32(255, 255, 255, 24));
            ImVec2 c(p.x + BTN_W * 0.5f, p.y + MENU_H * 0.5f);
            ImU32 col = IM_COL32(235, 235, 235, 255); float s = 5.0f;
            switch (glyph) {
                case 0: dl->AddLine(ImVec2(c.x-s,c.y), ImVec2(c.x+s,c.y), col, 1.0f); break;
                case 1: dl->AddRect(ImVec2(c.x-s,c.y-s), ImVec2(c.x+s,c.y+s), col, 0,0,1.0f); break;
                case 2:
                    dl->AddRect(ImVec2(c.x-s+2,c.y-s-2), ImVec2(c.x+s+2,c.y+s-2), col, 0,0,1.0f);
                    dl->AddRectFilled(ImVec2(c.x-s-2,c.y-s+2), ImVec2(c.x+s-2,c.y+s+2), IM_COL32(20,20,25,255));
                    dl->AddRect(ImVec2(c.x-s-2,c.y-s+2), ImVec2(c.x+s-2,c.y+s+2), col, 0,0,1.0f);
                    break;
                case 3:
                    dl->AddLine(ImVec2(c.x-s,c.y-s), ImVec2(c.x+s,c.y+s), col, 1.2f);
                    dl->AddLine(ImVec2(c.x-s,c.y+s), ImVec2(c.x+s,c.y-s), col, 1.2f);
                    break;
            }
            ImGui::SameLine(0, 0);
            return pressed;
        };
        if (ctrl("##min", 0, false)) window_driver.window_minimize(window);
        if (ctrl("##max", window_driver.window_is_maximized(window) ? 2 : 1, false)) window_driver.window_toggle_maximize(window);
        if (ctrl("##close", 3, true)) window.close_requested = true;

        ImGui::EndMenuBar();
    }

    const ImU32 bar_bg = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
    dl->AddRectFilled(ImVec2(origin.x, origin.y + MENU_H), ImVec2(origin.x + width, origin.y + H), bar_bg); 

    ImGui::SetCursorScreenPos(ImVec2(origin.x + LOGO + 6.0f, origin.y + MENU_H));
    const float TAB_PAD_Y = (TAB_H - ImGui::GetFontSize()) * 0.5f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14, TAB_PAD_Y));
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);
    if (ImGui::BeginTabBar("##TitlebarTabs", ImGuiTabBarFlags_AutoSelectNewTabs)) {
        for (int i = 0; i < (int)titlebar_tabs.size(); ++i) {
            ImGui::PushID(i);
            bool selected = ImGui::BeginTabItem(titlebar_tabs[i].c_str());
            blocker(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            if (selected) { titlebar_active_tab = i; ImGui::EndTabItem(); }
            ImGui::PopID();
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleVar(2);

    // {
    //     dl->PushClipRect(origin, ImVec2(origin.x + width, origin.y + H), false);
    //     float m = 6.0f;
    //     ImVec2 mn(origin.x + m, origin.y + m);
    //     ImVec2 mx(origin.x + LOGO - m, origin.y + H - m);
    //     dl->AddRectFilled(mn, mx, IM_COL32(255, 255, 255, 255), 4.0f);
    //     dl->PopClipRect();
    // }

    {
        VkDescriptorSet logo_set = imgui.texture_cache.get(logo_image.image_view);

        dl->PushClipRect(origin, ImVec2(origin.x + width, origin.y + H), false);
        float m = 6.0f;
        ImVec2 mn(origin.x + m, origin.y + m);
        ImVec2 mx(origin.x + LOGO - m, origin.y + H - m);
        if (logo_set) dl->AddImage(logo_set, mn, mx);
        else dl->AddRectFilled(mn, mx, IM_COL32(255, 255, 255, 255), 4.0f);
        dl->PopClipRect();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

}