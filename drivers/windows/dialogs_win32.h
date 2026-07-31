#pragma once
#include <string>

namespace ballistic::drivers {

std::wstring save_file_dialog_win32(const wchar_t* p_filter, const wchar_t* p_default_ext);
// std::wstring open_file_dialog_win32(const wchar_t* p_filter);

std::wstring open_folder_dialog_win32(const wchar_t* p_title);

}