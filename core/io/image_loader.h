#pragma once
#include <windows.h>
#include <string>

namespace ballistic {

struct ImageData
{
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
};

struct ImageLoader
{
    static ImageData load_from_resource(const std::wstring& p_resource_name);
    static ImageData load_from_file(const std::wstring& p_path);
    static ImageData load_from_icon(HICON p_icon);
    static void free_image(ImageData& p_image);
};

}