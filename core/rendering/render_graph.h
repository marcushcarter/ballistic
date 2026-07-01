#pragma once
#include <drivers/vulkan/rendering_device_driver_vulkan.h>
#include <core/log/error.h>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace ballistic {

struct RenderGraph;
    
enum class ResourceKind { Imported, Transient };

struct ImageResource {
    ResourceKind kind = ResourceKind::Imported;
    uint64_t name_id = 0;

    drivers::RenderingDeviceDriverVulkan::Image* image = nullptr;
    drivers::RenderingDeviceDriverVulkan::ImageDesc image_desc;

    VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t bindless_sampled = UINT32_MAX;
    uint32_t bindless_storage = UINT32_MAX;

    int producer = -1;
    bool read = false;
    bool written = false;
    int first_use = -1;
    int last_use = -1;
};

struct ImageAccess {
    uint64_t name_id = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    VkAccessFlags2 access = 0;
    bool is_write = false;
    int resource_index = -1;
};

struct RenderGraphBuilder {
    RenderGraph* graph = nullptr;
    uint32_t node_index = 0;

    void read_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
    void write_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
};

struct GraphPass {
    std::string name;
    std::function<void(RenderGraphBuilder&)> setup;
    std::function<void(VkCommandBuffer, RenderGraph&)> execute;
};

struct GraphImageBarrier {
    VkImage image = VK_NULL_HANDLE;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED, new_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlags2 src_stage = 0, dst_stage = 0;
    VkAccessFlags2 src_access = 0, dst_access = 0;
};

struct PassNode {
    GraphPass* pass = nullptr;
    std::vector<ImageAccess> image_accesses;
    std::vector<GraphImageBarrier> pre_barriers;
    bool culled = false;
};

struct RenderGraph
{
    drivers::RenderingDeviceDriverVulkan* device_driver = nullptr;

    std::unordered_map<uint64_t, std::string> debug_names;

    std::vector<ImageResource> image_resources;
    std::unordered_map<uint64_t, uint32_t> image_resource_map;
    std::vector<PassNode> nodes;
    std::vector<GraphImageBarrier> final_barriers;

    static uint64_t intern(std::string_view p_name);
    uint64_t intern_named(std::string_view p_name);

    drivers::RenderingDeviceDriverVulkan::Image* image(std::string_view p_name);
    ImageResource* resource(std::string_view p_name);
    ImageResource* resource_by_id(uint64_t p_name_id);

    void begin();
    void import_image(std::string_view p_name, drivers::RenderingDeviceDriverVulkan::Image* p_image, VkImageLayout p_final_layout);

    void add(GraphPass* p_pass);
    Error compile();
    void execute(VkCommandBuffer p_cmd);
};
    
}