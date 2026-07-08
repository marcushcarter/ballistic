#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/log/error.h>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace ballistic {

struct RenderGraph
{
    /***************/
    /**** SETUP ****/
    /***************/
    
    drivers::DeviceDriverVulkan* device_driver = nullptr;

    uint32_t frame_count = 1;
    uint32_t current_frame = 0;
    uint32_t width = 0, height = 0;
    
    Error create(drivers::DeviceDriverVulkan& r_device_driver, uint32_t frame_count);
    void destroy();
    Error set_size(uint32_t p_width, uint32_t p_height);
    
    static uint64_t intern(std::string_view p_name);
    uint64_t intern_named(std::string_view p_name);

    /*******************/
    /**** RESOURCES ****/
    /*******************/
    
    std::unordered_map<uint64_t, std::string> debug_names;
    
    enum class ResourceKind { Imported, Transient };

    void release_transients();
    
    // ----- IMAGE -----

    struct ImageResource {
        ResourceKind kind = ResourceKind::Imported;
        uint64_t name_id = 0;

        drivers::DeviceDriverVulkan::Image* image = nullptr;
        drivers::DeviceDriverVulkan::Image transient_storage;
        drivers::DeviceDriverVulkan::ImageCreateInfo image_create_info;

        VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 final_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        VkAccessFlags2 final_access = 0;

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

        bool is_attachment = false;
        bool is_depth = false;
        VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clear{};
    };

    struct ImageBarrier {
        VkImage image = VK_NULL_HANDLE;
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED, new_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 src_stage = 0, dst_stage = 0;
        VkAccessFlags2 src_access = 0, dst_access = 0;
    };

    struct ImageTransientPool {
        std::unordered_map<uint64_t, std::vector<drivers::DeviceDriverVulkan::Image>> free;
    };

    std::vector<ImageResource> image_resources;
    std::unordered_map<uint64_t, uint32_t> image_resource_map;
    std::vector<ImageBarrier> final_image_barriers;
    std::vector<ImageTransientPool> image_transient_pools;

    drivers::DeviceDriverVulkan::Image* image(std::string_view p_name);
    ImageResource* image_resource(std::string_view p_name);
    ImageResource* image_resource_by_id(uint64_t p_name_id);
    void import_image(std::string_view p_name, drivers::DeviceDriverVulkan::Image* p_image, VkImageLayout p_final_layout, VkPipelineStageFlags2 p_final_stage, VkAccessFlags2 p_final_access);
    void create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info);
    
    uint64_t _image_transient_key(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, VkExtent3D p_extent);
    void _image_resolve_extent(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, uint32_t& r_width, uint32_t& r_height);
    void _image_materialize_transient(ImageResource& r);
    void _image_release_transients();

    // ----- BUFFER -----

    // struct BufferResource {};
    // struct BufferAccess {};
    // struct BufferBarrier {};
    // struct BufferTransientPool {};
    
    // std::vector<BufferResource> buffer_resources;
    // std::unordered_map<uint64_t, uint32_t> buffer_resource_map;
    // std::vector<BufferBarrier> final_buffer_barriers;
    // std::vector<BufferTransientPool> buffer_transient_pools;
    
    // drivers::DeviceDriverVulkan::Buffer* buffer(std::string_view p_name);
    // BufferResource* buffer_resource(std::string_view p_name);
    // BufferResource* buffer_resource_by_id(uint64_t p_name_id);
    // void import_buffer();
    
    // uint64_t _buffer_transient_key(const drivers::DeviceDriverVulkan::BufferCreateInfo& p_create_info, VkExtent3D p_extent);
    // void _buffer_resolve_extent(const drivers::DeviceDriverVulkan::BufferCreateInfo& p_create_info, uint32_t& r_width, uint32_t& r_height);
    // void _buffer_materialize_transient(BufferResource& r);
    // void _buffer_release_transients();
    
    /**************/
    /**** PASS ****/
    /**************/

    struct Builder {
        RenderGraph* graph = nullptr;
        uint32_t node_index = 0;

        void create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info);
        void read_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
        void read_all_images(VkPipelineStageFlags2 p_stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
        void write_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
        void color_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear);
        void depth_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear);

        // void create_buffer();
        // void read_buffer();
        // void write_buffer();
        // void 
    };
    
    struct Pass {
        std::string name;
        std::function<void(Builder&)> setup;
        std::function<void(VkCommandBuffer, RenderGraph&)> execute;
    };

    struct Node {
        Pass* pass = nullptr;
        std::vector<ImageAccess> image_accesses;
        std::vector<ImageBarrier> pre_image_barriers;
        // std::vector<BufferAccess> image_accesses;
        // std::vector<BufferBarrier> pre_image_barriers;
        bool culled = false;

        bool has_render_pass = false;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkExtent2D area{};
        std::vector<VkClearValue> clear_values;
        std::vector<int> attachment_access_idx;
    };

    std::vector<Node> nodes;
    
    /***************/
    /**** CACHE ****/
    /***************/

    std::unordered_map<uint64_t, VkRenderPass> render_pass_cache;

    VkRenderPass _get_or_create_render_pass(Node& node);

    std::unordered_map<uint64_t, VkFramebuffer> framebuffer_cache;

    VkFramebuffer _get_or_create_framebuffer(Node& node);
    
    /***************/
    /**** GRAPH ****/
    /***************/

    void begin(uint32_t p_current_frame);
    void add(Pass* p_pass);
    Error compile();
    void execute(VkCommandBuffer p_cmd);
};
    
}