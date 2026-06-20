#include <core/io/image_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>

namespace ballistic {

static bool get_resource_bytes(const std::wstring& p_resource_name, const void*& p_data, DWORD& p_size)
{
    HMODULE h_module = GetModuleHandleW(nullptr);
    HRSRC h_res_info = FindResourceW(h_module, p_resource_name.c_str(), RT_RCDATA);
    if (!h_res_info) {
        return false;
    }
    HGLOBAL h_res_data = LoadResource(h_module, h_res_info);
    if (!h_res_data) {
        return false;
    }
    p_data = LockResource(h_res_data);
    p_size = SizeofResource(h_module, h_res_info);
    return p_data != nullptr && p_size > 0;
}
    
ImageData ImageLoader::load_from_resource(const std::wstring& p_resource_name)
{
    ImageData image;
    const void* raw_data = nullptr;
    DWORD raw_size = 0;
    if (!get_resource_bytes(p_resource_name, raw_data, raw_size)) {
        return image;
    }
    image.pixels = stbi_load_from_memory(
        static_cast<const unsigned char*>(raw_data), static_cast<int>(raw_size),
        &image.width, &image.height, &image.channels, 0
    );
    return image;
}

ImageData ImageLoader::load_from_file(const std::wstring& p_path)
{
    ImageData image;
    FILE* file = nullptr;
    errno_t err = _wfopen_s(&file, p_path.c_str(), L"rb");
    if (err != 0 || !file) {
        return image;
    }
    image.pixels = stbi_load_from_file(file, &image.width, &image.height, &image.channels, 0);
    fclose(file);
    return image;
}

ImageData ImageLoader::load_from_icon(HICON p_icon)
{
    ImageData image;

    ICONINFO icon_info{};
    if (!GetIconInfo(p_icon, &icon_info)) {
        return image;
    }

    BITMAP bitmap{};
    GetObject(icon_info.hbmColor, sizeof(bitmap), &bitmap);

    image.width = bitmap.bmWidth;
    image.height = bitmap.bmHeight;
    image.channels = 4;

    BITMAPINFOHEADER bmi{};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biWidth = image.width;
    bmi.biHeight = -image.height;
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biCompression = BI_RGB;

    image.pixels = static_cast<unsigned char*>(malloc(image.width * image.height * 4));

    HDC hdc = GetDC(nullptr);
    GetDIBits(hdc, icon_info.hbmColor, 0, image.height, image.pixels, reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);

    DeleteObject(icon_info.hbmColor);
    DeleteObject(icon_info.hbmMask);

    return image;
}

void ImageLoader::free_image(ImageData& p_image)
{
    if (p_image.pixels) {
        stbi_image_free(p_image.pixels);
        p_image.pixels = nullptr;
    }
}

}