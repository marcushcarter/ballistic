#include <core/rendering/render_graph.h>
#include <core/log/error_macros.h>

namespace ballistic {

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

void RenderGraphBuilder::read_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = p_layout;
    a.stage = p_stage;
    a.access = p_access;
    a.is_write = false;
    graph->nodes[node_index].image_accesses.push_back(a);
}

void RenderGraphBuilder::write_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access)
{
    ImageAccess a;
    a.name_id = graph->intern_named(p_name);
    a.layout = p_layout;
    a.stage = p_stage;
    a.access = p_access;
    a.is_write = true;
    graph->nodes[node_index].image_accesses.push_back(a);
}

drivers::RenderingDeviceDriverVulkan::Image* RenderGraph::image(std::string_view p_name)
{
    auto it = image_resource_map.find(intern(p_name));
    if (it == image_resource_map.end()) return nullptr;
    return image_resources[it->second].image;
}

ImageResource* RenderGraph::resource(std::string_view p_name)
{
    auto it = image_resource_map.find(intern(p_name));
    if (it == image_resource_map.end()) return nullptr;
    return &image_resources[it->second];
}

ImageResource* RenderGraph::resource_by_id(uint64_t p_name_id)
{
    auto it = image_resource_map.find(p_name_id);
    if (it == image_resource_map.end()) return nullptr;
    return &image_resources[it->second];
}

void RenderGraph::begin()
{
    image_resources.clear();
    image_resource_map.clear();
    nodes.clear();
    final_barriers.clear();
}

void RenderGraph::import_image(std::string_view p_name, drivers::RenderingDeviceDriverVulkan::Image* p_image, VkImageLayout p_final_layout)
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

void RenderGraph::add(GraphPass* p_pass)
{
    PassNode node;
    node.pass = p_pass;
    uint32_t idx = static_cast<uint32_t>(nodes.size());
    nodes.push_back(std::move(node));

    if (p_pass->setup) {
        RenderGraphBuilder builder{ this, idx };
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
            if (a.is_write) { r.written = true; r.producer = static_cast<int>(n); }
            else            { r.read = true; }
        }
    }

    for (ImageResource& r : image_resources) {
        bool is_sink = (r.kind == ResourceKind::Imported) && r.final_layout != VK_IMAGE_LAYOUT_UNDEFINED;
        if (r.written && !r.read && !is_sink)
            log_write("RenderGraph: '%s' written but never read (dead write).", debug_names[r.name_id].c_str());
        if (r.read && !r.written && r.kind != ResourceKind::Imported)
            log_write("RenderGraph: '%s' read before write.", debug_names[r.name_id].c_str());
    }

    for (PassNode& node : nodes) node.culled = true;
    std::vector<uint32_t> worklist;
    for (uint32_t n = 0; n < nodes.size(); ++n) {
        for (ImageAccess& a : nodes[n].image_accesses) {
            if (a.is_write && a.resource_index >= 0) {
                ImageResource& r = image_resources[a.resource_index];
                bool is_sink = (r.kind == ResourceKind::Imported) && r.final_layout != VK_IMAGE_LAYOUT_UNDEFINED;
                if (is_sink && nodes[n].culled) { nodes[n].culled = false; worklist.push_back(n); }
            }
        }
    }
    while (!worklist.empty()) {
        uint32_t n = worklist.back(); worklist.pop_back();
        for (ImageAccess& a : nodes[n].image_accesses) {
            if (!a.is_write && a.resource_index >= 0) {
                int prod = image_resources[a.resource_index].producer;
                if (prod >= 0 && nodes[prod].culled) { nodes[prod].culled = false; worklist.push_back((uint32_t)prod); }
            }
        }
    }

    for (PassNode& node : nodes) {
        if (node.culled) continue;
        for (ImageAccess& a : node.image_accesses) {
            if (a.resource_index < 0) continue;
            ImageResource& r = image_resources[a.resource_index];
            auto& img = *r.image;

            if (img.state.layout != a.layout) {
                GraphImageBarrier b{};
                b.image = img.image;
                b.aspect = img.aspect;
                b.old_layout = img.state.layout;
                b.new_layout = a.layout;
                b.src_stage = img.state.stage;
                b.dst_stage = a.stage;
                b.src_access = img.state.access;
                b.dst_access = a.access;
                node.pre_barriers.push_back(b);
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

        GraphImageBarrier b{};
        b.image = img.image;
        b.aspect = img.aspect;
        b.old_layout = img.state.layout;
        b.new_layout = r.final_layout;
        b.src_stage = img.state.stage;
        b.dst_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        b.src_access = img.state.access;
        b.dst_access = 0;
        final_barriers.push_back(b);

        img.state.layout = r.final_layout;
    }

    return Ok;
}

static void emit_barriers(VkCommandBuffer p_cmd, const std::vector<GraphImageBarrier>& p_barriers)
{
    if (p_barriers.empty()) return;

    std::vector<VkImageMemoryBarrier2> vk_barriers;
    vk_barriers.reserve(p_barriers.size());
    for (const GraphImageBarrier& b : p_barriers) {
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
    for (PassNode& node : nodes) {
        if (node.culled) continue;
        emit_barriers(p_cmd, node.pre_barriers);
        if (node.pass->execute) node.pass->execute(p_cmd, *this);
    }

    emit_barriers(p_cmd, final_barriers);
}

}