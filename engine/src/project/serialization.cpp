#include "serialization.h"
#include "project.h"

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

std::string FormatToString(VkFormat format)
{
    switch (format) {
        #define X(f) case f: return #f;
        RG_IMAGE_FORMATS
        #undef X
        default: return "VK_FORMAT_R8G8B8A8_UNORM";
    }
}

VkFormat StringToFormat(const std::string& s)
{
    #define X(f) if (s == #f) return f;
    RG_IMAGE_FORMATS
    #undef X
    return VK_FORMAT_R8G8B8A8_UNORM;
}

bool Serialize(const Project& project)
{
    if (project.path.empty()) return false;
 
    try {
        toml::table root;
        root.insert("name", project.name);
        root.insert("engine_version", project.engineVersion);

        toml::array images;
        for (const auto& d : project.images) {
            toml::table t;
            t.insert("id", (int64_t)d.id);
            t.insert("name", d.name);
            t.insert("format", FormatToString(d.format));
            t.insert("size_mode", d.sizeMode == ImageSizeMode::Fixed ? "fixed" : "viewport_relative");
            t.insert("relative_width", (double)d.relativeWidth);
            t.insert("relative_height", (double)d.relativeHeight);
            t.insert("fixed_width", (int64_t)d.fixedWidth);
            t.insert("fixed_height", (int64_t)d.fixedHeight);
            t.insert("usage_attachment", d.usageAttachment);
            t.insert("usage_sampled", d.usageSampled);
            t.insert("usage_storage", d.usageStorage);
            images.push_back(std::move(t));
        }
        root.insert("images", images);

        toml::array passes;
        for (const auto& p : project.passes) {
            toml::table t;
            t.insert("id", (int64_t)p.id);
            t.insert("name", p.name);
            t.insert("engine_hook", p.engineHook);
            t.insert("enabled", p.enabled);
 
            toml::array reads, writes;
            for (uint64_t id : p.reads) reads.push_back((int64_t)id);
            for (uint64_t id : p.writes) writes.push_back((int64_t)id);
            t.insert("reads", reads);
            t.insert("writes", writes);
            passes.push_back(std::move(t));
        }
        root.insert("passes", passes);
 
        toml::array passOrder;
        for (uint64_t id : project.graph.passOrder) passOrder.push_back((int64_t)id);
        toml::table graph;
        graph.insert("pass_order", passOrder);
        root.insert("graph", graph);
 
        std::ofstream f(project.path / "project.blst");
        f << root;
        return true;
    } catch (...) {
        LOG_ERROR("Failed to save project.blst");
        return false;
    }
}

bool Deserialize(Project& project)
{
    const std::filesystem::path blstPath = project.path / "project.blst";
    if (!std::filesystem::exists(blstPath)) {
        LOG_ERROR("project.blst not found: %s", blstPath.string().c_str());
        return false;
    }
 
    try {
        auto root = toml::parse_file(blstPath.string());
        project.name = root["name"].value_or<std::string>("");
        project.engineVersion = root["engine_version"].value_or<std::string>("");
 
        project.images.clear();
        project.passes.clear();
        project.graph.passOrder.clear();

        const toml::array* imageArr = root["images"].as_array();
        toml::table legacyRoot;
        if (!imageArr || imageArr->empty()) {
            std::filesystem::path legacy = project.path / "render_graph.blst";
            if (std::filesystem::exists(legacy)) {
                legacyRoot = toml::parse_file(legacy.string());
                imageArr = legacyRoot["images"].as_array();
                LOG_WARN("Migrated images from legacy render_graph.blst; resave to consolidate");
            }
        }

        if (imageArr) {
            imageArr->for_each([&](auto& el){
                if (!el.is_table()) return;
                const toml::table& t = *el.as_table();
                ResourceImageDesc d;
                d.id = (uint64_t)t["id"].value_or<int64_t>(0);
                d.name = t["name"].value_or<std::string>("");
                d.format = StringToFormat(t["format"].value_or<std::string>("VK_FORMAT_R8G8B8A8_UNORM"));
                d.sizeMode = t["size_mode"].value_or<std::string>("") == "fixed" ? ImageSizeMode::Fixed : ImageSizeMode::ViewportRelative;
                d.relativeWidth = (float)t["relative_width"].value_or<double>(1.0);
                d.relativeHeight  = (float)t["relative_height"].value_or<double>(1.0);
                d.fixedWidth = (uint32_t)t["fixed_width"].value_or<int64_t>(0);
                d.fixedHeight = (uint32_t)t["fixed_height"].value_or<int64_t>(0);
                d.usageAttachment = t["usage_attachment"].value_or(true);
                d.usageSampled = t["usage_sampled"].value_or(true);
                d.usageStorage = t["usage_storage"].value_or(false);
                if (d.id != 0 && !d.name.empty()) project.images.push_back(d);
            });
        }

        if (auto* arr = root["passes"].as_array()) {
            arr->for_each([&](auto& el){
                if (!el.is_table()) return;
                const toml::table& t = *el.as_table();
                RGPass p;
                p.id = (uint64_t)t["id"].value_or<int64_t>(0);
                p.name = t["name"].value_or<std::string>("");
                p.engineHook = t["engine_hook"].value_or<std::string>("");
                p.enabled = t["enabled"].value_or(true);
                if (auto* r = t["reads"].as_array()) r->for_each([&](auto& e){ if (e.is_integer()) p.reads.push_back((uint64_t)e.as_integer()->get()); });
                if (auto* w = t["writes"].as_array()) w->for_each([&](auto& e){ if (e.is_integer()) p.writes.push_back((uint64_t)e.as_integer()->get()); });
                if (p.id != 0) project.passes.push_back(p);
            });
        }

        if (auto* graph = root["graph"].as_table()) {
            if (auto* arr = (*graph)["pass_order"].as_array())
                arr->for_each([&](auto& e){ if (e.is_integer()) project.graph.passOrder.push_back((uint64_t)e.as_integer()->get()); });
        }
 
        return true;
    } catch (const toml::parse_error& e) {
        LOG_ERROR("Failed to parse project.blst: %s", e.what());
        return false;
    }
}
