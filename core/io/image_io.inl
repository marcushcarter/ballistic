#include <stb_image.h>
#include <stb_image_write.h>
#include <cstdio>
#include <cstdlib>

inline bool get_resource_bytes(const std::wstring& p_resource_name, const void*& p_data, DWORD& p_size)
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

inline void stbi_write_file_cb(void* p_context, void* p_data, int p_size)
{
    std::fwrite(p_data, 1, static_cast<size_t>(p_size), static_cast<FILE*>(p_context));
}

template<typename T, int Channels>
ImageData<T, Channels> ImageIO::load_from_file(const std::wstring& p_path)
{
    ImageData<T, Channels> image;
    FILE* file = nullptr;
    if (_wfopen_s(&file, p_path.c_str(), L"rb") != 0 || !file) return image;
    image.pixels = reinterpret_cast<T*>(stbi_load_from_file(file, &image.width, &image.height, &image.source_channels, Channels));
    fclose(file);
    return image;
}

template<typename T, int Channels>
ImageData<T, Channels> ImageIO::load_from_resource(const std::wstring& p_resource_name)
{
    ImageData<T, Channels> image;
    const void* raw_data = nullptr;
    DWORD raw_size = 0;
    if (!get_resource_bytes(p_resource_name, raw_data, raw_size)) return image;
    image.pixels = reinterpret_cast<T*>(stbi_load_from_memory(static_cast<const unsigned char*>(raw_data), static_cast<int>(raw_size), &image.width, &image.height, &image.source_channels, Channels));
    return image;
}

template<typename T, int Channels>
void ImageIO::free_image(ImageData<T, Channels>& p_image)
{
    if (p_image.pixels) {
        stbi_image_free(p_image.pixels);
        p_image.pixels = nullptr;
    }
}

template<typename T, int Channels>
bool ImageIO::save_png(const std::wstring& p_path, const ImageData<T, Channels>& p_image)
{
    if (!p_image.pixels || p_image.width <= 0 || p_image.height <= 0) return false;
    FILE* file = nullptr;
    if (_wfopen_s(&file, p_path.c_str(), L"wb") != 0 || !file) return false;
    int stride = p_image.width * Channels;
    int result = stbi_write_png_to_func(stbi_write_file_cb, file, p_image.width, p_image.height, Channels, p_image.pixels, stride);
    fclose(file);
    return result != 0;
}
