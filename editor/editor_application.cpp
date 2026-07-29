#include <editor/editor_application.h>
#include <editor/popup/editor_settings/editor_settings.h>
#include <editor/popup/project_settings/project_settings.h>
#include <editor/popup/export/export.h>
#include <core/io/embedded_resource.h>
#include <core/io/path.h>
#include <core/io/image_io.h>
#include <core/io/path.h>
#include <core/math/color.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <windows.h>
#include <shellapi.h>
#include <filesystem>

namespace ballistic {

Error EditorApplication::on_init()
{
    using enum Error;

    ImageData<uint8_t, 4> logo = ImageIO::load_from_resource<uint8_t, 4>(L"LOGOS_ICON_PNG");
    if (logo.valid()) logo_image = dd.image_create_texture(logo.pixels, static_cast<uint32_t>(logo.width), static_cast<uint32_t>(logo.height), "editor_logo");
    ImageIO::free_image(logo);

    Error err = win32.window_set_icon(EmbeddedResource::load_icon(L"BALLISTIC_ICON"));
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = win32.window_set_titlebar_color(RGB(20, 20, 25));
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
    
    popups.push_back(std::make_unique<EditorSettingsPopup>());
    popups.push_back(std::make_unique<ProjectSettingsPopup>());
    popups.push_back(std::make_unique<ExportPopup>());

    close_project();

    _load_state();
    settings.theme.apply();

    // if (settings.restore_project_on_load && !settings.restore_project_root.empty() && std::filesystem::exists(settings.restore_project_root)) {
    //     open_project(settings.restore_project_root);
    // }

    return Ok;
}

void EditorApplication::on_update(float p_dt)
{
    _draw_titlebar();

    EditorContext ctx{};
    ctx.win32 = &win32;
    ctx.imgui = &imgui;
    ctx.renderer = &renderer;
    ctx.render_path = static_cast<EditorRenderPath*>(render_path);
    ctx.project = &project;
    ctx.settings = &settings;

    for (auto& p : popups) p->draw(ctx);

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
    _save_state();
    project_manager.save_recent();
}

void EditorApplication::_load_state()
{
    std::ifstream f(Paths::roaming_data() / "editor_state.cfg");
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        size_t sp = line.rfind(' ');
        if (sp == std::string::npos) continue;
        std::string key = line.substr(0, sp), val = line.substr(sp + 1);

        if (key == "interface.theme.preset") { settings.theme.preset = Theme::theme_preset_index(val); continue; }
        if (key == "interface.theme.base") { color_from_hex(val, settings.theme.base); continue; }
        if (key == "interface.theme.accent") { color_from_hex(val, settings.theme.accent); continue; }
        if (key == "interface.theme.use_system_accent") { settings.theme.use_system_accent = std::atoi(val.c_str()) != 0; continue; }

        if (key == "interface.window.custom_titlebar") { win32.window_set_custom_titlebar(std::atoi(val.c_str()) != 0); continue; }

        if (key == "debugger.profiler.enabled") { renderer.graph.profiler.enabled = std::atoi(val.c_str()) != 0; continue; }

        if (key.size() > 5 && key.compare(key.size() - 5, 5, ".open") == 0) editor.panel_open[key.substr(0, key.size() - 5)] = std::atoi(val.c_str()) != 0;
    }

    settings.theme.apply();
}

void EditorApplication::_save_state()
{
    std::ofstream f(Paths::roaming_data() / "editor_state.cfg");
    if (!f) return;

    f << "interface.theme.preset " << Theme::theme_preset_name(settings.theme.preset) << '\n';
    f << "interface.theme.base " << color_to_hex(settings.theme.base) << '\n';
    f << "interface.theme.accent " << color_to_hex(settings.theme.accent) << '\n';
    f << "interface.theme.use_system_accent " << (settings.theme.use_system_accent ? 1 : 0) << '\n';
    
    f << "interface.window.custom_titlebar " << (win32.window.custom_titlebar ? 1 : 0) << '\n';

    f << "debugger.profiler.enabled " << (renderer.graph.profiler.enabled ? 1 : 0) << '\n';

    for (const auto& [name, open] : editor.panel_open) f << name << ".open " << (open ? 1 : 0) << '\n';
}
    
void EditorApplication::open_project(const std::filesystem::path& p_root)
{
    if (project.load(p_root) != Error::Ok) return;

    project_manager.add_recent(p_root, project.name);
    render_path_request(new EditorRenderPath());
    mode = Mode::Editor;

    win32.window_set_title(project.name + std::string(" - Ballistic Editor"));
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

    win32.window_set_title("Ballistic Editor - Project Manager");
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

    win32.window_titlebar_reset((int)H);
    auto blocker = [&](ImVec2 mn, ImVec2 mx) {
        win32.window_titlebar_add_rect((long)(mn.x - origin.x), (long)(mn.y - origin.y), (long)(mx.x - origin.x), (long)(mx.y - origin.y));
    };

    if (ImGui::BeginMenuBar()) {
        
        ImGui::SetCursorPosX(LOGO + 6.0f);
        float menu_x0 = ImGui::GetCursorScreenPos().x;

        if (editor_created && mode == Mode::Editor) _draw_shared_menu_items();
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Online Documentation")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            // if (ImGui::MenuItem("Forum")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            // if (ImGui::MenuItem("Community")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            ImGui::Separator();
            if (ImGui::MenuItem("Copy System Info")) {

            }
            ImGui::Separator();
            // if (ImGui::MenuItem("About Ballistic")) open_popup("About Ballistic");
            // if (ImGui::MenuItem("Support Development")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            ImGui::EndMenu();
        }

        float menu_x1 = ImGui::GetCursorScreenPos().x;
        if (menu_x1 > menu_x0) blocker(ImVec2(menu_x0, origin.y), ImVec2(menu_x1, origin.y + MENU_H));

        const std::string& title = project.name.empty() ? std::string("Ballistic Editor") : project.name;
        ImVec2 ts = ImGui::CalcTextSize(title.c_str());
        float btns_x = origin.x + width - BTN_W * 3.0f;
        float right_pad = win32.window.custom_titlebar ? (width - BTN_W * 3.0f) : width;
        float title_x = origin.x + right_pad - ts.x - 16.0f;
        dl->AddText(ImVec2(title_x, origin.y + (MENU_H - ts.y) * 0.5f), IM_COL32(200, 200, 205, 255), title.c_str());
        
        if (win32.window.custom_titlebar) {
            float cluster_x = btns_x;
            float cluster_w = BTN_W * 3.0f;
            dl->AddRectFilled(
                ImVec2(cluster_x, origin.y),
                ImVec2(cluster_x + cluster_w, origin.y + MENU_H),
                IM_COL32(38, 38, 44, 255));

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
            if (ctrl("##min", 0, false)) win32.window_minimize();
            if (ctrl("##max", win32.window_is_maximized() ? 2 : 1, false)) win32.window_toggle_maximize();
            if (ctrl("##close", 3, true)) win32.window.close_requested = true;
        }

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

    {
        const char* cog = ICON_FA_GEAR;
        ImVec2 cog_sz = ImGui::CalcTextSize(cog);
        float pad = 12.0f;
        float box = TAB_H;
        ImVec2 cog_pos(origin.x + width - pad - box, origin.y + MENU_H);

        ImGui::SetCursorScreenPos(cog_pos);
        ImGui::InvisibleButton("##settings_cog", ImVec2(box, box));
        blocker(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) open_popup("Editor Settings");

        ImU32 col = hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 205, 255);
        dl->AddText(ImVec2(cog_pos.x + (box - cog_sz.x) * 0.5f, cog_pos.y + (box - cog_sz.y) * 0.5f), col, cog);
    }

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

void EditorApplication::_draw_shared_menu_items()
{
    if (ImGui::BeginMenu("Project")) {
        if (ImGui::MenuItem("Project Settings")) open_popup("Project Settings");
        ImGui::Separator();
        if (ImGui::MenuItem("Version Control")) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Export")) open_popup("Export");
        if (ImGui::MenuItem("Pack Project as ZIP")) {
            // file dialog
            // export project
            // convert to zip
            // delete exported folder
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit to Project List")) editor.close_project_requested = true;
        if (ImGui::MenuItem("Quit")) win32.window_request_close();
        ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("Scene")) {
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Editor")) {
        if (ImGui::MenuItem("Editor Settings")) open_popup("Editor Settings");
        ImGui::Separator();
        if (ImGui::MenuItem("Take Screenshot")) {
            EditorRenderPath* path = static_cast<EditorRenderPath*>(render_path);
            path->screenshot.requested = true;
        }
        if (ImGui::MenuItem("Toggle Fullscreen")) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Open Editor Data Folder")) Paths::reveal_in_explorer(Paths::roaming_data());
        ImGui::EndMenu();
    }
    
    editor.draw_menu();
}

void EditorApplication::open_popup(std::string_view name)
{
    for (auto& p : popups) if (name == p->name()) { p->open = true; return; }
}

}