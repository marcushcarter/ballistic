#pragma once
#include <Ballistic.h>

namespace Ballistic {

    class EditorTheme {
    public:
        EditorTheme() {
            LoadDefault();
            Apply();
        }

        void LoadDefault();

        // void LoadFromFile(std::string);
        // void SaveToFile(const char* path);

        void PushColor(ImGuiCol_ widget, size_t colorIndex, float alpha = 1.0f) {
            ImVec4 col = m_palette[colorIndex];
            col.w = alpha;
            ImGui::PushStyleColor(widget, col);
            m_colorPushCount++;
        }

        void PopColors() {
            ImGui::PopStyleColor(m_colorPushCount);
            m_colorPushCount = 0;
        }
        
        void Apply();

        ImVec4& Color(size_t index) { return m_palette[index]; }
    
    private:
        std::array<ImVec4, 100> m_palette;
        int m_colorPushCount = 0;
        // int m_stylePushCount = 0;
    };

}