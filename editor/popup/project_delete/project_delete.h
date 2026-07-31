#pragma once
#include <editor/popup/popup.h>
#include <filesystem>
#include <string>

namespace ballistic {

struct DeleteProjectPopup : Popup
{
    const char* name() const override { return "Delete Project"; }
    ImVec2 initial_size() const override { return ImVec2(500, 125); }
    void draw_contents(EditorContext& ctx) override;
    void draw_footer(EditorContext& ctx) override;

    std::filesystem::path project_path;
    std::string project_name;
};

}