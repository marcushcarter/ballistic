#pragma once
#include <editor/popup/popup.h>
#include <functional>
#include <string>

namespace ballistic {

struct ConfirmPopup : Popup
{
    const char* name() const override { return "##confirm"; }

    void draw_contents(EditorContext& ctx) override;
    void draw_footer(EditorContext& ctx) override;

    // Call then set open = true. The message + button label describe the action;
    // on_confirm runs if the user clicks the confirm button.
    void configure(std::string p_title, std::string p_message, std::string p_confirm_label, std::function<void()> p_on_confirm)
    {
        title = std::move(p_title);
        message = std::move(p_message);
        confirm_label = std::move(p_confirm_label);
        on_confirm = std::move(p_on_confirm);
    }

    std::string title;
    std::string message;
    std::string confirm_label = "OK";
    std::function<void()> on_confirm;
};

}