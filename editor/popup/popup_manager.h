#pragma once
#include <editor/popup/popup.h>
#include <editor/popup/confirm/confirm.h>
#include <editor/editor_context.h>
#include <vector>
#include <memory>
#include <string_view>

namespace ballistic {

struct PopupManager
{
    std::vector<std::unique_ptr<Popup>> popups;
    ConfirmPopup confirm_popup;

    void register_popup(std::unique_ptr<Popup> p);
    void open(std::string_view p_name);
    void draw(EditorContext& ctx);

    void confirm(std::string title, std::string message, std::string confirm_label, std::function<void()> on_confirm);
};
    
}