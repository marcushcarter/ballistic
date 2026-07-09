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
    current_frame = 0;
    image_transient_pools.clear();
    image_transient_pools.resize(frame_count);
    // buffer_transient_pools.clear();
    // buffer_transient_pools.resize(frame_count);

    return Ok;
}

void RenderGraph::destroy()
{
    device_driver->device_wait_idle();

    for (auto& [k, fb] : framebuffer_cache) device_driver->framebuffer_free(fb);
    framebuffer_cache.clear();
    for (auto& [k, rp] : render_pass_cache) device_driver->render_pass_free(rp);
    render_pass_cache.clear();

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

    for (auto& [k, fb] : framebuffer_cache) device_driver->framebuffer_free(fb);
    framebuffer_cache.clear();

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

void RenderGraph::import_image(std::string_view p_name, drivers::DeviceDriverVulkan::Image* p_image, VkImageLayout p_final_layout, VkPipelineStageFlags2 p_final_stage, VkAccessFlags2 p_final_access)
{
    uint64_t id = intern_named(p_name);

    ImageResource r{};
    r.kind = ResourceKind::Imported;
    r.name_id = id;
    r.image = p_image; 
    r.final_layout = p_final_layout;
    r.final_stage = p_final_stage;
    r.final_access = p_final_access;

    p_image->state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    p_image->state.stage  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    p_image->state.access = 0;

    uint32_t res_idx = static_cast<uint32_t>(image_resources.size());
    image_resources.push_back(r);
    auto [it, inserted] = image_resource_map.insert({ id, res_idx });
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
    ImageTransientPool& pool = image_transient_pools[current_frame];

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
    ImageTransientPool& pool = image_transient_pools[current_frame];
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

void RenderGraph::Builder::create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info) { graph->create_image(p_name, p_create_info); }

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

void RenderGraph::Builder::read_all_images(VkPipelineStageFlags2 p_stage)
{
    Node& node = graph->nodes[node_index];
    for (RenderGraph::ImageResource& r : graph->image_resources) {
        if (r.final_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) continue;

        bool already = false;
        for (ImageAccess& existing : node.image_accesses) if (existing.name_id == r.name_id) { already = true; break; }
        if (already) continue;

        ImageAccess a;
        a.name_id  = r.name_id;
        a.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        a.stage = p_stage;
        a.access = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        a.is_write = false;
        node.image_accesses.push_back(a);
    }
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

void RenderGraph::Builder::color_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    a.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    a.access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    a.is_write = true;
    a.is_attachment = true;
    a.load_op = p_load;
    a.clear = p_clear;
    graph->nodes[node_index].image_accesses.push_back(a);
}

void RenderGraph::Builder::depth_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    a.stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    a.access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    a.is_write = true;
    a.is_attachment = true;
    a.is_depth = true;
    a.load_op = p_load;
    a.clear = p_clear;
    graph->nodes[node_index].image_accesses.push_back(a);
}

/***************/
/**** CACHE ****/
/***************/

VkRenderPass RenderGraph::_get_or_create_render_pass(Node& node)
{
    drivers::DeviceDriverVulkan::RenderPassCreateInfo ci{};
    ci.attachments.reserve(node.attachment_access_idx.size());

    uint64_t key = 1469598103934665603ull;
    auto mix = [&](uint64_t v){ key ^= v; key *= 1099511628211ull; };

    for (int ai : node.attachment_access_idx) {
        ImageAccess& a = node.image_accesses[ai];
        auto& img = *image_resources[a.resource_index].image;

        drivers::DeviceDriverVulkan::RenderPassCreateInfo::Attachment att{};
        att.format = img.format;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.load_op = a.load_op;
        att.store_op = a.store_op;
        att.initial_layout = (a.load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? img.state.layout : VK_IMAGE_LAYOUT_UNDEFINED;
        att.final_layout = a.layout;
        att.is_depth = a.is_depth;
        ci.attachments.push_back(att);

        mix((uint64_t)att.format);
        mix((uint64_t)att.load_op);
        mix((uint64_t)att.store_op);
        mix((uint64_t)att.initial_layout);
        mix((uint64_t)att.final_layout);
        mix((uint64_t)att.is_depth);
    }

    if (auto it = render_pass_cache.find(key); it != render_pass_cache.end()) return it->second;

    ci.name = "graph_render_pass";
    VkRenderPass rp = device_driver->render_pass_create(ci);
    render_pass_cache[key] = rp;
    return rp;
}

VkRenderPass RenderGraph::acquire_render_pass(const Pass& p_pass)
{
    // Compatibility keys only on format + is_depth (order matters).
    // Salt distinguishes these canonical compat passes from node passes,
    // which also fold in load/store/layout.
    uint64_t key = 1469598103934665603ull;
    auto mix = [&](uint64_t v){ key ^= v; key *= 1099511628211ull; };
    mix(0xC0FFEEull);
    for (const Pass::Format& f : p_pass.formats) {
        mix((uint64_t)f.format);
        mix((uint64_t)f.is_depth);
    }

    if (auto it = render_pass_cache.find(key); it != render_pass_cache.end()) return it->second;

    drivers::DeviceDriverVulkan::RenderPassCreateInfo ci{};
    ci.attachments.reserve(p_pass.formats.size());
    for (const Pass::Format& f : p_pass.formats) {
        drivers::DeviceDriverVulkan::RenderPassCreateInfo::Attachment att{};
        att.format = f.format;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // irrelevant to compatibility
        att.store_op = VK_ATTACHMENT_STORE_OP_STORE;
        att.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.final_layout = f.is_depth ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                                      : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.is_depth = f.is_depth;
        ci.attachments.push_back(att);
    }
    ci.name = "compat_render_pass";
    VkRenderPass rp = device_driver->render_pass_create(ci);
    render_pass_cache[key] = rp;
    return rp;
}

VkFramebuffer RenderGraph::_get_or_create_framebuffer(Node& node)
{
    std::vector<VkImageView> views;
    views.reserve(node.attachment_access_idx.size());

    uint64_t key = 1469598103934665603ull;
    auto mix = [&](uint64_t v) { key ^= v; key *= 1099511628211ull; };
    mix((uint64_t)(uintptr_t)node.render_pass);

    for (int ai : node.attachment_access_idx) {
        auto& img = *image_resources[node.image_accesses[ai].resource_index].image;
        views.push_back(img.image_view);
        mix((uint64_t)(uintptr_t)img.image_view);
    }

    if (auto it = framebuffer_cache.find(key); it != framebuffer_cache.end()) return it->second;

    VkFramebuffer fb = device_driver->framebuffer_create(node.render_pass, views, node.area);
    framebuffer_cache[key] = fb;
    return fb;
}

/***************/
/**** GRAPH ****/
/***************/

void RenderGraph::begin(uint32_t p_current_frame)
{
    current_frame = p_current_frame;
    
    image_resources.clear();
    image_resources.reserve(64);

    // image_resources.clear();
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
        Builder builder{ this, p_pass, idx };
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
        node.attachment_access_idx.clear();

        for (int i = 0; i < (int)node.image_accesses.size(); ++i) {
            ImageAccess& a = node.image_accesses[i];
            if (a.resource_index < 0) continue;
            ImageResource& r = image_resources[a.resource_index];
            if (r.kind == ResourceKind::Transient && !r.image) _image_materialize_transient(r);
            auto& img = *r.image;

            if (a.is_attachment) {
                node.attachment_access_idx.push_back(i);
                if (node.area.width == 0) node.area = { img.extent.width, img.extent.height };
                continue;
            }

            constexpr VkAccessFlags2 WRITE_MASK =
                VK_ACCESS_2_SHADER_WRITE_BIT |
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_2_TRANSFER_WRITE_BIT |
                VK_ACCESS_2_HOST_WRITE_BIT |
                VK_ACCESS_2_MEMORY_WRITE_BIT;

            const bool layout_change = img.state.layout != a.layout;
            const bool prev_write = (img.state.access & WRITE_MASK) != 0;
            const bool curr_write = a.is_write || (a.access & WRITE_MASK) != 0;

            if (layout_change || prev_write || curr_write) {
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

        if (!node.attachment_access_idx.empty()) {
            node.has_render_pass = true;
            node.render_pass = _get_or_create_render_pass(node);
            node.framebuffer = _get_or_create_framebuffer(node);

            node.clear_values.clear();
            node.clear_values.reserve(node.attachment_access_idx.size());
            for (int ai : node.attachment_access_idx) node.clear_values.push_back(node.image_accesses[ai].clear);

            for (int ai : node.attachment_access_idx) {
                ImageAccess& a = node.image_accesses[ai];
                auto& img = *image_resources[a.resource_index].image;
                img.state.layout = a.layout;
                img.state.stage  = a.stage;
                img.state.access = a.access;
            }
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
        b.dst_stage = r.final_stage;
        b.src_access = img.state.access;
        b.dst_access = r.final_access;
        final_image_barriers.push_back(b);

        img.state.layout = r.final_layout;
        img.state.stage = r.final_stage;
        img.state.access = r.final_access;
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

        if (node.has_render_pass) {
            VkRenderPassBeginInfo bi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            bi.renderPass      = node.render_pass;
            bi.framebuffer     = node.framebuffer;
            bi.renderArea      = { {0,0}, node.area };
            bi.clearValueCount = (uint32_t)node.clear_values.size();
            bi.pClearValues    = node.clear_values.data();
            vkCmdBeginRenderPass(p_cmd, &bi, VK_SUBPASS_CONTENTS_INLINE);
        }

        if (node.pass->execute) node.pass->execute(p_cmd, *this);
        if (node.has_render_pass) vkCmdEndRenderPass(p_cmd);
    }

    emit_barriers(p_cmd, final_image_barriers);
    release_transients();
}

}