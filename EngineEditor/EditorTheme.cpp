#include "EditorTheme.h"

namespace Ballistic {
    
    void EditorTheme::LoadDefault() {
        m_palette[0] = ImVec4(1,1,1,1); // what this color is used for
        Apply();
    }

    void EditorTheme::Apply() {
        ImGuiStyle& style = ImGui::GetStyle();

		// style.Colors[ImGuiCol_WindowBg] = m_palette[0];
        // style.Colors[ImGuiCol_PopupBg] = m_palette[0];
		// style.Colors[ImGuiCol_Border] = m_palette[0];
		// style.Colors[ImGuiCol_Header] = m_palette[0];
		// style.Colors[ImGuiCol_HeaderHovered] = m_palette[0];
		// style.Colors[ImGuiCol_HeaderActive] = m_palette[0];
		// style.Colors[ImGuiCol_Button] = m_palette[0];
		// style.Colors[ImGuiCol_ButtonHovered] = m_palette[0];
		// style.Colors[ImGuiCol_ButtonActive] = m_palette[0];
		// style.Colors[ImGuiCol_CheckMark] = m_palette[0];
		// style.Colors[ImGuiCol_SliderGrab] = m_palette[0];
		// style.Colors[ImGuiCol_SliderGrabActive] = m_palette[0];
		// style.Colors[ImGuiCol_FrameBg] = m_palette[0];
		// style.Colors[ImGuiCol_FrameBgHovered] = m_palette[0];
		// style.Colors[ImGuiCol_FrameBgActive] = m_palette[0];
		// style.Colors[ImGuiCol_Tab] = m_palette[0];
		// style.Colors[ImGuiCol_TabHovered] = m_palette[0];
		// style.Colors[ImGuiCol_TabActive] = m_palette[0];

		// style.Colors[ImGuiCol_TabSelectedOverline] = m_palette[0];
		// style.Colors[ImGuiCol_TabDimmedSelectedOverline] = m_palette[0];
		// style.Colors[ImGuiCol_TabUnfocused] = m_palette[0];
		// style.Colors[ImGuiCol_TabUnfocusedActive] = m_palette[0];
		// style.Colors[ImGuiCol_TableRowBg] = m_palette[0];
		// style.Colors[ImGuiCol_TableRowBgAlt] = m_palette[0];
		// style.Colors[ImGuiCol_TitleBg] = m_palette[0];
		// style.Colors[ImGuiCol_TitleBgActive] = m_palette[0];
		// style.Colors[ImGuiCol_TitleBgCollapsed] = m_palette[0];
		// style.Colors[ImGuiCol_ScrollbarGrab] = m_palette[0];
		// style.Colors[ImGuiCol_ResizeGrip] = m_palette[0];
		// style.Colors[ImGuiCol_ResizeGripHovered] = m_palette[0];
		// style.Colors[ImGuiCol_ResizeGripActive] = m_palette[0];
		// style.Colors[ImGuiCol_Separator] = m_palette[0];
		// style.Colors[ImGuiCol_SeparatorHovered] = m_palette[0];
		// style.Colors[ImGuiCol_SeparatorActive] = m_palette[0];
		// style.Colors[ImGuiCol_Text] = m_palette[0];
		// style.Colors[ImGuiCol_TextDisabled] = m_palette[0];
		// style.Colors[ImGuiCol_MenuBarBg] = m_palette[0];
    }
    
}