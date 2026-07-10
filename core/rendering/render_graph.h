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
    
    drivers::DeviceDriverVulkan* dd = nullptr;

    uint32_t frame_count = 1;
    uint32_t current_frame = 0;
    uint32_t width = 0, height = 0;
    
    Error create(drivers::DeviceDriverVulkan& r_dd, uint32_t frame_count);
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
    uint64_t _image_transient_key(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, VkExtent2D p_extent);
    void _image_resolve_extent(const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info, uint32_t& r_width, uint32_t& r_height);
    void _image_materialize_transient(ImageResource& r);
    void _image_release_transients();

    // ----- BUFFER -----

    struct BufferResource {
        ResourceKind kind = ResourceKind::Imported;
        uint64_t name_id = 0;

        drivers::DeviceDriverVulkan::Buffer* buffer = nullptr;
        drivers::DeviceDriverVulkan::Buffer transient_storage;
        drivers::DeviceDriverVulkan::BufferCreateInfo buffer_create_info;

        VkPipelineStageFlags2 final_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        VkAccessFlags2 final_access = 0;

        int producer = -1;
        bool read = false;
        bool written = false;
        int first_use = -1;
        int last_use = -1;
    };

    struct BufferAccess {
        uint64_t name_id = 0;
        VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        VkAccessFlags2 access = 0;
        bool is_write = false;
        int resource_index = -1;
    };

    struct BufferBarrier {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkDeviceSize size = VK_WHOLE_SIZE;
        VkPipelineStageFlags2 src_stage = 0, dst_stage = 0;
        VkAccessFlags2 src_access = 0, dst_access = 0;
    };

    struct BufferTransientPool {
        std::unordered_map<uint64_t, std::vector<drivers::DeviceDriverVulkan::Buffer>> free;
    };
    
    std::vector<BufferResource> buffer_resources;
    std::unordered_map<uint64_t, uint32_t> buffer_resource_map;
    std::vector<BufferBarrier> final_buffer_barriers;
    std::vector<BufferTransientPool> buffer_transient_pools;
    
    drivers::DeviceDriverVulkan::Buffer* buffer(std::string_view p_name);
    BufferResource* buffer_resource(std::string_view p_name);
    BufferResource* buffer_resource_by_id(uint64_t p_name_id);
    void import_buffer(std::string_view p_name, drivers::DeviceDriverVulkan::Buffer* p_buffer, VkPipelineStageFlags2 p_final_stage, VkAccessFlags2 p_final_access);
    
    void create_buffer(std::string_view p_name, const drivers::DeviceDriverVulkan::BufferCreateInfo& p_create_info);
    uint64_t _buffer_transient_key(VkBufferUsageFlags p_usage, drivers::DeviceDriverVulkan::BufferCreateInfo::Memory p_memory, VkDeviceSize p_capacity);
    void _buffer_materialize_transient(BufferResource& r);
    void _buffer_release_transients();
    
    /**************/
    /**** PASS ****/
    /**************/

    struct Pass;

    struct Builder {
        RenderGraph* graph = nullptr;
        Pass* pass = nullptr;
        uint32_t node_index = 0;

        void create_image(std::string_view p_name, const drivers::DeviceDriverVulkan::ImageCreateInfo& p_create_info);
        void read_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
        void read_all_images(VkPipelineStageFlags2 p_stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
        void write_image(std::string_view p_name, VkImageLayout p_layout, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
        void color_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear);
        void depth_attachment(std::string_view p_name, VkAttachmentLoadOp p_load, VkClearValue p_clear);

        void create_buffer(std::string_view p_name, const drivers::DeviceDriverVulkan::BufferCreateInfo& p_create_info);
        void read_buffer(std::string_view p_name, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
        void write_buffer(std::string_view p_name, VkPipelineStageFlags2 p_stage, VkAccessFlags2 p_access);
    };
    
    struct Pass {
        std::string name;
        
        struct Format { VkFormat format = VK_FORMAT_UNDEFINED; bool is_depth = false; };
        std::vector<Format> formats;

        std::function<void(Builder&)> setup;
        std::function<void(VkCommandBuffer, RenderGraph&)> execute;
    };

    struct Node {
        Pass* pass = nullptr;
        std::vector<ImageAccess> image_accesses;
        std::vector<ImageBarrier> pre_image_barriers;
        std::vector<BufferAccess> buffer_accesses;
        std::vector<BufferBarrier> pre_buffer_barriers;
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
    VkRenderPass acquire_render_pass(const Pass& p_pass);

    std::unordered_map<uint64_t, VkFramebuffer> framebuffer_cache;

    VkFramebuffer _get_or_create_framebuffer(Node& node);

    /******************/
    /**** PROFILER ****/
    /******************/

    struct PassTiming {
        uint64_t name_id = 0;
        const char* name = "?";
        double gpu_ms = 0.0;
    };

    struct Profiler {
        bool enabled = false;
        double period_ns = 1.0;
        uint64_t valid_mask = ~0ull;

        std::vector<drivers::DeviceDriverVulkan::QueryPool> pools;
        std::vector<std::vector<uint64_t>> slot_names;
        std::vector<uint32_t> slot_count;
        std::vector<uint8_t> slot_recorded;

        std::vector<PassTiming> last_results;
        double last_total_ms = 0.0;

        uint32_t query_count = 0;
        static constexpr uint32_t CAPACITY = 128;
    };

    Profiler profiler;

    void _profiler_resolve();
    void _profiler_frame_begin(VkCommandBuffer p_cmd);
    void _profiler_mark(VkCommandBuffer p_cmd, const std::string& p_name);
    void _profiler_frame_end();
    
    void profiler_initialize();
    void profiler_shutdown();
    
    /***************/
    /**** GRAPH ****/
    /***************/

    void begin(uint32_t p_current_frame);
    void add(Pass* p_pass);
    Error compile();
    void execute(VkCommandBuffer p_cmd);
};
    
}