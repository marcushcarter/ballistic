#include <editor/popup/popup_manager.h>

namespace ballistic {

void PopupManager::register_popup(std::unique_ptr<Popup> p)
{
    popups.push_back(std::move(p));
}

void PopupManager::open(std::string_view p_name)
{
    for (auto& p : popups) {
        if (p_name == p->name()) {
            p->open = true;
            return;
        }
    }
}

void PopupManager::confirm(std::string title, std::string message, std::string confirm_label, std::function<void()> on_confirm)
{
    confirm_popup.configure(std::move(title), std::move(message), std::move(confirm_label), std::move(on_confirm));
    confirm_popup.open = true;
}

void PopupManager::draw(EditorContext& ctx)
{
    for (auto& p : popups) p->draw(ctx);
    confirm_popup.draw(ctx);
}

}