#include "Panels/ConsolePanel.h"

namespace ballistic
{
    ConsolePanel::ConsolePanel(const std::string& name) 
        : IPanel(name) {}
    
    void ConsolePanel::OnAttach() {

    }
    
    void ConsolePanel::OnDetach() {
    }

    void ConsolePanel::OnUpdate(float deltaTime) {
        auto& logManager = *GetRoot()->GetLogManager();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(300, 200),   // minimum size
            ImVec2(FLT_MAX, FLT_MAX) // maximum size (or any limit you want)
        );

        static ImGuiWindowFlags ConsoleFlags = ImGuiWindowFlags_NoCollapse;
        ImGui::Begin((const char*)u8"\uF120 Console", nullptr, ConsoleFlags);

        ImGui::PushItemWidth(-1);
        ImGui::InputTextWithHint("##filter", "Filter logs...", filterBuffer, sizeof(filterBuffer));
        ImGui::PopItemWidth();

        ImGui::BeginChild("LogsChild", ImVec2(0, 0), false);

        ImVec2 childSize = ImGui::GetContentRegionAvail();

        float checkboxWidth = 100.0f;
        ImGui::BeginChild("Checkboxes", ImVec2(checkboxWidth, childSize.y), true);

        for (int i = 0; i < 4; i++) {
            auto level = static_cast<LogLevel>(i);
            bool enabled = logManager.IsLevelEnabled(level);
            if (ImGui::Checkbox(logManager.LevelToString(level), &enabled)) {
                logManager.SetLevelEnabled(level, enabled);
            }
        }
        if (ImGui::Button("Clear")) logManager.Clear();
        if (ImGui::Button("Copy")) {
            std::string logs = logManager.GetFilteredLogsText(filterBuffer);
            ImGui::SetClipboardText(logs.c_str());
        }

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("LogsArea", ImVec2(childSize.x - checkboxWidth, childSize.y), true, ImGuiWindowFlags_HorizontalScrollbar);

        auto logs = logManager.GetFilteredLogs(filterBuffer);
        for (const auto& msg : logs) {
            ImVec4 color;
            switch (msg.level) {
                case LogLevel::Debug:   color = ImVec4(0.3f,0.3f,0.3f,1.0f); break;
                case LogLevel::Info:    color = ImVec4(1.0f,1.0f,1.0f,1.0f); break;
                case LogLevel::Warning: color = ImVec4(1.0f,1.0f,0.0f,1.0f); break;
                case LogLevel::Error:   color = ImVec4(1.0f,0.0f,0.0f,1.0f); break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextWrapped("[%s] %s", logManager.LevelToString(msg.level), msg.message.c_str());
            ImGui::PopStyleColor();
        }

        if (scrollToBottom)
            ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;

        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ConsolePanel::OnEvent(IEvent& e) {

    }

} // namespace ballistic
