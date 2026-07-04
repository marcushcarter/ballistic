#include <core/rendering/render_graph.h>
#include <core/log/error_macros.h>

namespace ballistic {

/***************/
/**** SETUP ****/
/***************/
    
Error RenderGraph::create(drivers::DeviceDriverVulkan& r_device_driver, uint32_t p_frame_count)
{
    using enum Error;

    device_driver = &r_device_driver;
    frame_count = p_frame_count;
    frame_index = 0;
    image_transient_pools.clear();
    image_transient_pools.resize(frame_count);
    // buffer_transient_pools.clear();
    // buffer_transient_pools.resize(frame_count);

    return Ok;
}

void RenderGraph::destroy()
{
    device_driver->device_wait_idle();

    for (ImageTransientPool& pool : image_transient_pools) {
        for (auto& [key, imgs] : pool.free)
            for (drivers::DeviceDriverVulkan::Image& img : imgs) device_driver->image_free(img);
        pool.free.clear();
    }
    image_transient_pools.clear();
}

Error RenderGraph::set_size(uint32_t p_width, uint32_t p_height)
{
    using enum Error;

    if (p_width == 0 || p_height == 0) return Ok;
    if (p_width == width && p_height == height) return Ok;
    width = p_width;
    height = p_height;

    for (ImageTransientPool& pool : image_transient_pools) {
        for (auto& [key, imgs] : pool.free)
            for (drivers::DeviceDriverVulkan::Image& img : imgs) device_driver->image_free(img);
        pool.free.clear();
    }

    return Ok;
}

uint64_t RenderGraph::intern(std::string_view p_name)
{
    uint64_t hash = 1469598103934665603ull;
    for (char c : p_name) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ull;
    }
    return hash;
}

uint64_t RenderGraph::intern_named(std::string_view p_name)
{
    uint64_t id = intern(p_name);
    if (!debug_names.contains(id)) debug_names.emplace(id, std::string(p_name));
    return id;
}

/*******************/
/**** RESOURCES ****/
/*******************/

void RenderGraph::release_transients()
{
    _image_release_transients();
    // _buffer_release_transients();
}

// ----- IMAGE -----

drivers::DeviceDriverVulkan::Image* RenderGraph::image(std::string_view p_name)
{
    auto it = image_resource_map.find(intern(p_name));
    if (it == image_resource_map.end()) return nullptr;
    return image_resources[it->second].image;
}

RenderGraph::ImageResource* RenderGraph::image_resource(std::string_view p_name)
{
    auto it = image_resource_map.find(intern(p_name));
    if (it == image_resource_map.end()) return nullptr;
    return &image_resources[it->second];
}

RenderGraph::ImageResource* RenderGraph::image_resource_by_id(uint64_t p_name_id)
{
    auto it = image_resource_map.find(p_name_id);
    if (it == image_resource_map.end()) return nullptr;
    return &image_resources[it->second];
}

void RenderGraph::import_image(std::string_view p_name, drivers::DeviceDriverVulkan::Image* p_image, VkImageLayout p_final_layout)
{
    uint64_t id = intern_named(p_name);

    ImageResource r{};
    r.kind = ResourceKind::Imported;
    r.name_id = id;
    r.image = p_image; 
    r.final_layout = p_final_layout;

    uint32_t res_idx = static_cast<uint32_t>(image_resources.size());
    image_resources.push_back(r);
    image_resource_map[id] = res_idx;
}

void RenderGraph::create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info)
{
    uint64_t id = intern_named(p_name);
    if (image_resource_map.contains(id)) return;

    ImageResource r{};
    r.kind = ResourceKind::Transient;
    r.name_id = id;
    r.image = nullptr;
    r.image_create_info = p_create_info;
    r.final_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    uint32_t idx = static_cast<uint32_t>(image_resources.size());
    image_resources.push_back(r);
    image_resource_map[id] = idx;    
}

uint64_t RenderGraph::_image_transient_key(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, VkExtent3D p_extent)
{
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](uint64_t v) { h ^= v; h *= 1099511628211ull; };
    mix(p_create_info.format);
    mix(p_create_info.usage);
    mix(p_create_info.aspect);
    mix(p_create_info.samples);
    mix(p_create_info.mip_levels);
    mix(p_create_info.layers);
    mix(p_extent.width);
    mix(p_extent.height);
    mix(p_extent.depth);
    return h;
}

void RenderGraph::_image_resolve_extent(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, uint32_t& r_width, uint32_t& r_height)
{
    using Sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing;
    if (p_create_info.sizing == Sizing::Fixed) {
        r_width = p_create_info.fixed_width;
        r_height = p_create_info.fixed_height;
    } else {
        r_width = std::max(1u, static_cast<uint32_t>(std::lround(width * p_create_info.width_scale)));
        r_height = std::max(1u, static_cast<uint32_t>(std::lround(height * p_create_info.height_scale)));
    }
}

void RenderGraph::_image_materialize_transient(ImageResource& r)
{
    if (r.image) return;
    
    drivers::DeviceDriverVulkan::ImageCreateInfo image_ci = r.image_create_info;
    image_ci.name = debug_names[r.name_id].c_str();

    uint32_t w, h;
    _image_resolve_extent(image_ci, w, h);
    VkExtent3D extent{ w, h, 1 };

    uint64_t key = _image_transient_key(image_ci, extent);
    ImageTransientPool& pool = image_transient_pools[frame_index];

    drivers::DeviceDriverVulkan::Image img;
    auto it = pool.free.find(key);
    if (it != pool.free.end() && !it->second.empty()) {
        img = it->second.back();
        it->second.pop_back();
    } else {
        img = device_driver->image_create_dedicated(image_ci, extent);
    }

    img.state = {};

    r.transient_storage = img;
    r.image = &r.transient_storage;
}

void RenderGraph::_image_release_transients()
{
    if (image_transient_pools.empty()) return;
    ImageTransientPool& pool = image_transient_pools[frame_index];
    for (ImageResource& r : image_resources) {
        if (r.kind != ResourceKind::Transient) continue;
        if (!r.image) continue;

        uint64_t key = _image_transient_key(r.image_create_info, r.transient_storage.extent);
        pool.free[key].push_back(r.transient_storage);

        r.image = nullptr;
        r.transient_storage = {};
    }
}

// ----- BUFFER -----

/**************/
/**** PASS ****/
/**************/

// ----- BUILDER -----

void RenderGraph::Builder::read_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = p_layout;
    a.stage = p_stage;
    a.access = p_access;
    a.is_write = false;
    graph->nodes[node_index].image_accesses.push_back(a);
}

void RenderGraph::Builder::write_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = p_layout;
    a.stage = p_stage;
    a.access = p_access;
    a.is_write = true;
    graph->nodes[node_index].image_accesses.push_back(a);
}

void RenderGraph::Builder::create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info) { graph->create_image(p_name, p_create_info); }

// ----- GRAPH -----

void RenderGraph::begin()
{
    // recycle heap

    image_resources.clear();
    image_resource_map.clear();
    final_image_barriers.clear();
    // buffer_resources.clear();
    // buffer_resource_map.clear();
    // final_buffer_barriers.clear();
    nodes.clear();
}

void RenderGraph::add(Pass* p_pass)
{
    Node node;
    node.pass = p_pass;
    uint32_t idx = static_cast<uint32_t>(nodes.size());
    nodes.push_back(std::move(node));

    if (p_pass->setup) {
        Builder builder{ this, idx };
        p_pass->setup(builder);
    }
}

Error RenderGraph::compile()
{
    using enum Error;

    for (uint32_t n = 0; n < nodes.size(); ++n) {
        for (ImageAccess& a : nodes[n].image_accesses) {
            auto it = image_resource_map.find(a.name_id);
            if (it == image_resource_map.end()) {
                log_write("RenderGraph: pass '%s' accesses unimported image '%s'.", nodes[n].pass->name.c_str(), debug_names[a.name_id].c_str());
                continue;
            }

            a.resource_index = static_cast<int>(it->second);
            ImageResource& r = image_resources[a.resource_index];
            if (a.is_write) {
                r.written = true;
                r.producer = static_cast<int>(n);
            } else {
                r.read = true;
            }
        }
    }

    for (ImageResource& r : image_resources) {
        bool is_sink = (r.kind == ResourceKind::Imported) && r.final_layout != VK_IMAGE_LAYOUT_UNDEFINED;
        if (r.written && !r.read && !is_sink) log_write("RenderGraph: '%s' written but never read (dead write).", debug_names[r.name_id].c_str());
        if (r.read && !r.written && r.kind != ResourceKind::Imported) log_write("RenderGraph: '%s' read before write.", debug_names[r.name_id].c_str());
    }

    for (Node& node : nodes) node.culled = true;
    std::vector<uint32_t> worklist;
    for (uint32_t n = 0; n < nodes.size(); ++n) {
        for (ImageAccess& a : nodes[n].image_accesses) {
            if (a.is_write && a.resource_index >= 0) {
                ImageResource& r = image_resources[a.resource_index];
                bool is_sink = (r.kind == ResourceKind::Imported) && r.final_layout != VK_IMAGE_LAYOUT_UNDEFINED;
                if (is_sink && nodes[n].culled) {
                    nodes[n].culled = false;
                    worklist.push_back(n);
                }
            }
        }
    }
    while (!worklist.empty()) {
        uint32_t n = worklist.back();
        worklist.pop_back();
        for (ImageAccess& a : nodes[n].image_accesses) {
            if (!a.is_write && a.resource_index >= 0) {
                int prod = image_resources[a.resource_index].producer;
                if (prod >= 0 && nodes[prod].culled) {
                    nodes[prod].culled = false;
                    worklist.push_back((uint32_t)prod);
                }
            }
        }
    }

    for (Node& node : nodes) {
        if (node.culled) continue;
        for (ImageAccess& a : node.image_accesses) {
            if (a.resource_index < 0) continue;
            ImageResource& r = image_resources[a.resource_index];
            if (r.kind == ResourceKind::Transient && !r.image) _image_materialize_transient(r);
            auto& img = *r.image;

            if (img.state.layout != a.layout) {
                ImageBarrier b{};
                b.image = img.image;
                b.aspect = img.aspect;
                b.old_layout = img.state.layout;
                b.new_layout = a.layout;
                b.src_stage = img.state.stage;
                b.dst_stage = a.stage;
                b.src_access = img.state.access;
                b.dst_access = a.access;
                node.pre_image_barriers.push_back(b);
            }
            img.state.layout = a.layout;
            img.state.stage = a.stage;
            img.state.access = a.access;
        }
        
    }

    for (ImageResource& r : image_resources) {
        if (r.final_layout == VK_IMAGE_LAYOUT_UNDEFINED) continue;
        auto& img = *r.image;
        if (img.state.layout == r.final_layout) continue;

        ImageBarrier b{};
        b.image = img.image;
        b.aspect = img.aspect;
        b.old_layout = img.state.layout;
        b.new_layout = r.final_layout;
        b.src_stage = img.state.stage;
        b.dst_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        b.src_access = img.state.access;
        b.dst_access = 0;
        final_image_barriers.push_back(b);

        img.state.layout = r.final_layout;
    }

    return Ok;
}

static void emit_barriers(VkCommandBuffer p_cmd, const std::vector<RenderGraph::ImageBarrier>& p_barriers)
{
    if (p_barriers.empty()) return;

    std::vector<VkImageMemoryBarrier2> vk_barriers;
    vk_barriers.reserve(p_barriers.size());
    for (const RenderGraph::ImageBarrier& b : p_barriers) {
        VkImageMemoryBarrier2 mb{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        mb.srcStageMask = b.src_stage;
        mb.srcAccessMask = b.src_access;
        mb.dstStageMask = b.dst_stage;
        mb.dstAccessMask = b.dst_access;
        mb.oldLayout = b.old_layout;
        mb.newLayout = b.new_layout;
        mb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        mb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        mb.image = b.image;
        mb.subresourceRange = { b.aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
        vk_barriers.push_back(mb);
    }

    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.imageMemoryBarrierCount = static_cast<uint32_t>(vk_barriers.size());
    dep.pImageMemoryBarriers = vk_barriers.data();
    vkCmdPipelineBarrier2(p_cmd, &dep);
}

void RenderGraph::execute(VkCommandBuffer p_cmd)
{
    for (Node& node : nodes) {
        if (node.culled) continue;
        emit_barriers(p_cmd, node.pre_image_barriers);
        if (node.pass->execute) node.pass->execute(p_cmd, *this);
    }

    emit_barriers(p_cmd, final_image_barriers);

    release_transients();
}

}