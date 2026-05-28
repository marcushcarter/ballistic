#include "render_graph_io.h"

#define RG_IMAGE_FORMATS \
    X(VK_FORMAT_R8G8B8A8_UNORM) \
    X(VK_FORMAT_R8G8B8A8_SRGB) \
    X(VK_FORMAT_B8G8R8A8_UNORM) \
    X(VK_FORMAT_R16G16B16A16_SFLOAT) \
    X(VK_FORMAT_R32G32B32A32_SFLOAT) \
    X(VK_FORMAT_R32G32_SFLOAT) \
    X(VK_FORMAT_R16G16_SFLOAT) \
    X(VK_FORMAT_R32_SFLOAT) \
    X(VK_FORMAT_R8_UNORM) \
    X(VK_FORMAT_D32_SFLOAT) \
    X(VK_FORMAT_D16_UNORM) \
    X(VK_FORMAT_X8_D24_UNORM_PACK32) \
    X(VK_FORMAT_D16_UNORM_S8_UINT) \
    X(VK_FORMAT_D24_UNORM_S8_UINT) \
    X(VK_FORMAT_D32_SFLOAT_S8_UINT)

std::string RGFormatToString(VkFormat format)
{
    switch (format) {
        #define X(f) case f: return #f;
        RG_IMAGE_FORMATS
        #undef X
        default: return "VK_FORMAT_R8G8B8A8_UNORM";
    }
}

VkFormat RGStringToFormat(const std::string& s)
{
    #define X(f) if (s == #f) return f;
    RG_IMAGE_FORMATS
    #undef X
    return VK_FORMAT_R8G8B8A8_UNORM;
}

bool LoadRenderGraph(const std::filesystem::path& blstPath, std::vector<RGImage>& outImages)
{
    try {
        auto rg = toml::parse_file(blstPath.string());
        auto arr = rg["images"].as_array();
        if (!arr) return true;

        arr->for_each([&](auto& el) {
            if (!el.is_table()) return;
            auto& t = *el.as_table();

            RGImage img;
            img.id = t["id"].value_or<uint64_t>(0);
            img.name = t["name"].value_or<std::string>("");
            img.format = RGStringToFormat(t["format"].value_or<std::string>("VK_FORMAT_R8G8B8A8_UNORM"));
            img.sizeMode = t["size_mode"].value_or<std::string>("") == "fixed" ? RGImageSizeMode::Fixed : RGImageSizeMode::ViewportRelative;
            img.relativeWidth = (float)t["relative_width"].value_or<double>(1.0);
            img.relativeHeight = (float)t["relative_height"].value_or<double>(1.0);
            img.fixedWidth = t["fixed_width"].value_or<uint32_t>(0);
            img.fixedHeight = t["fixed_height"].value_or<uint32_t>(0);
            img.usageAttachment = t["usage_attachment"].value_or(true);
            img.usageSampled = t["usage_sampled"].value_or(true);
            img.usageStorage = t["usage_storage"].value_or(false);

            if (img.id != 0 && !img.name.empty()) outImages.push_back(img);
        });
        return true;
    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse render_graph.blst: %s", e.what());
        return false;
    }
}

bool SaveRenderGraph(const std::filesystem::path& blstPath, const std::vector<RGImage>& images)
{
    try {
        toml::table rg;
        toml::array arr;
        for (auto& img : images) {
            toml::table entry;
            entry.insert("id", (int64_t)img.id);
            entry.insert("name", img.name);
            entry.insert("format", RGFormatToString(img.format));
            entry.insert("size_mode", img.sizeMode == RGImageSizeMode::Fixed ? "fixed" : "viewport_relative");
            entry.insert("relative_width", (double)img.relativeWidth);
            entry.insert("relative_height", (double)img.relativeHeight);
            entry.insert("fixed_width", (int64_t)img.fixedWidth);
            entry.insert("fixed_height", (int64_t)img.fixedHeight);
            entry.insert("usage_attachment", img.usageAttachment);
            entry.insert("usage_sampled", img.usageSampled);
            entry.insert("usage_storage", img.usageStorage);
            arr.push_back(entry);
        }
        rg.insert("images", arr);
        std::ofstream f(blstPath);
        f << rg;
        return true;
    } catch (...) {
        LOG_ERROR("Failed to save render_graph.blst");
        return false;
    }
}
