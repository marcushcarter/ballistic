#pragma once

#include <drivers/vulkan/context_driver_vulkan.h>

#include <shaderc/shaderc.hpp>
#include <vma/vk_mem_alloc.h>

#include <drivers/vulkan/ballistic_vulkan.h>

#include <string>

namespace ballistic::drivers {

struct DeviceDriverVulkan
{
    /***************/
    /**** SETUP ****/
    /***************/
	
    struct Queue {
		VkQueue queue = VK_NULL_HANDLE;
	};

    struct SubgroupCapabilities {
        uint32_t size = 0;
        VkShaderStageFlags supported_stages = 0;
        VkSubgroupFeatureFlags supported_operations = 0;
    };

    VkDevice device = VK_NULL_HANDLE;
    ContextDriverVulkan* cd = nullptr;
    uint32_t device_index = 0;
    DriverDevice driver_device;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t frame_count = 1;
	VkPhysicalDeviceProperties physical_device_properties = {};
	VkPhysicalDeviceFeatures physical_device_features = {};
	VkPhysicalDeviceFeatures requested_device_features = {};
    std::unordered_map<std::string, bool> requested_device_extensions;
    std::unordered_set<std::string> enabled_device_extension_names;
    std::vector<std::vector<Queue>> queue_families;
    SubgroupCapabilities subgroup_capabilities;

	void _register_requested_device_extension(const std::string& p_extension_name, bool p_required);
    Error _initialize_device_extensions();
    void _get_device_properties();
    Error _check_device_features();
    Error _check_device_capabilities();
    Error _add_queue_create_info(std::vector<VkDeviceQueueCreateInfo> &r_queue_create_info);
    Error _initialize_device(const std::vector<VkDeviceQueueCreateInfo> &p_queue_create_info);
    Error _initialize_allocator();
    Error _initialize_pipeline_cache();
    void _check_subgroup_capabilities();

    Error initialize(ContextDriverVulkan& r_cd, uint32_t p_device_index, uint32_t p_frame_count);
    void shutdown();

    Error device_wait_idle();

    /****************/
    /**** MEMORY ****/
    /****************/

    VmaAllocator allocator = nullptr;
    
	/****************/
	/**** IMAGES ****/
	/****************/

    struct ImageBarrierState {
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        VkAccessFlags2 access = 0;
    };

    struct ImageCreateInfo {
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageUsageFlags usage = 0;
        VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        uint32_t mip_levels = 1, layers = 1;

        enum class Sizing { ViewportRelative, Fixed };
        Sizing sizing = Sizing::ViewportRelative;
        float width_scale = 1.0f, height_scale = 1.0f;
        uint32_t fixed_width = 0, fixed_height = 0;
        
        const char* name = nullptr;
    };

    struct Image {
        VkImage image = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        ImageBarrierState state;

        uint32_t bindless_sampled = UINT32_MAX;
        uint32_t bindless_storage = UINT32_MAX;

        VkExtent2D extent = {};
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t mip_levels = 1, layers = 1;
        VkMemoryRequirements mem_req = {};
    };

    Image _image_create(const ImageCreateInfo& p_create_info, VkExtent2D p_extent);
    Error _image_bind(Image& r_image, VmaAllocation p_allocation);
    Error _image_create_view(Image& r_image);

    Image image_create_dedicated(const ImageCreateInfo& p_create_info, VkExtent2D p_extent);
    void image_free(Image& r_image);
    
	/*****************/
	/**** BUFFERS ****/
	/*****************/
    
    // ----- BUFFER -----

    struct BufferBarrierState {
        VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        VkAccessFlags2 access = 0;
    };

    struct BufferCreateInfo {
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = 0;

        enum class Memory { DeviceLocal, HostVisible };
        Memory memory = Memory::DeviceLocal;
        const char* name = nullptr;
    };

    struct Buffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        BufferBarrierState state;

        VkDeviceSize capacity = 0;
        VkDeviceSize size = 0;
        VkDeviceSize device_address = 0;
        void* mapped = nullptr;

        VkBufferUsageFlags usage = 0;
        BufferCreateInfo::Memory memory = BufferCreateInfo::Memory::DeviceLocal;
        const char* name = nullptr;
    };

    VkDeviceSize _next_power_of_2(VkDeviceSize v);

    Buffer buffer_create(const BufferCreateInfo& p_create_info);
    void buffer_free(Buffer& r_buffer);
    Error buffer_ensure_capacity(Buffer& r_buffer, VkDeviceSize p_size);
    Error buffer_update(Buffer& r_buffer, const void* p_data, VkDeviceSize p_size, VkDeviceSize p_offset = 0);
    // void buffer_update_staged(VkCommandBuffer p_cmd, Buffer& r_dst, Buffer& r_staging, const void* p_data, VkDeviceSize p_size, VkDeviceSize p_dst_offset = 0, VkDeviceSize p_staging_offset = 0);

    // ----- RING -----

    struct BufferRing {
        std::vector<Buffer> buffers;
    };

    BufferRing buffer_ring_create(const BufferCreateInfo& p_create_info, uint32_t p_frame_count);
    void buffer_ring_free(BufferRing& r_buffer_ring);

    /*****************/
    /**** SAMPLER ****/
    /*****************/

    struct SamplerCreateInfo {
        VkFilter filter = VK_FILTER_LINEAR;
        VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        float anisotropy = 1.0f;

        bool compare = false;
        VkCompareOp compare_op = VK_COMPARE_OP_LESS;

        const char* name = nullptr;
    };

    struct Sampler {
        VkSampler sampler = VK_NULL_HANDLE;
        uint32_t bindless_sampler = UINT32_MAX;
    };

    Sampler default_sampler;

    Sampler sampler_create(const SamplerCreateInfo& p_create_info);
    void sampler_free(Sampler& r_sampler);

    /****************/
    /**** FENCES ****/
    /****************/

    VkFence fence_create(bool p_signaled = true);
    void fence_free(VkFence& r_fence);
    Error fence_wait(VkFence p_fence, uint64_t p_timeout = UINT64_MAX);
    Error fence_reset(VkFence p_fence);

    /********************/
    /**** SEMAPHORES ****/
    /********************/

    VkSemaphore semaphore_create();
    void semaphore_free(VkSemaphore& r_semaphore);

    /***************/
    /**** QUERY ****/
    /***************/

    struct QueryPool {
        VkQueryPool pool = VK_NULL_HANDLE;
        uint32_t capacity = 0;
    };

    uint32_t timestamp_valid_bits(uint32_t p_queue_family_index);

    QueryPool query_pool_create_timestamp(uint32_t p_query_count);
    QueryPool query_pool_create_occlusion(uint32_t p_query_count);
    QueryPool query_pool_create_pipeline_statistics(uint32_t p_query_count, VkQueryPipelineStatisticFlags p_stats);
    void query_pool_free(QueryPool& r_query_pool);
    Error query_pool_get_results(const QueryPool& p_query_pool, uint32_t p_first, uint32_t p_count, uint64_t* r_results, uint32_t p_stride_u64 = 1);

    void command_reset_query_pool(VkCommandBuffer p_cmd, const QueryPool& p_query_pool, uint32_t p_first, uint32_t p_count);
    void command_write_timestamp(VkCommandBuffer p_cmd, const QueryPool& p_query_pool, VkPipelineStageFlags2 p_stage, uint32_t p_index);
    void command_begin_query(VkCommandBuffer p_cmd, const QueryPool& p_query_pool, uint32_t p_index, VkQueryControlFlags p_flags = 0);
    void command_end_query(VkCommandBuffer p_cmd, const QueryPool& p_query_pool, uint32_t p_index);

    /******************/
    /**** COMMANDS ****/
    /******************/

    struct CommandPool {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    };

    CommandPool command_pool_create(uint32_t p_queue_family_index, VkCommandBufferLevel p_buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void command_pool_free(CommandPool& r_cmd_pool);
    Error command_pool_reset(CommandPool& r_cmd_pool);

    VkCommandBuffer command_buffer_create(CommandPool& p_cmd_pool);
    Error command_buffer_begin(VkCommandBuffer p_cmd_buffer, VkCommandBufferUsageFlags p_flags = 0);
    Error command_buffer_end(VkCommandBuffer p_cmd_buffer);
    Error swapchain_update();

    void command_render_set_viewport(VkCommandBuffer p_cmd, const std::vector<VkRect2D>& p_viewports);
    void command_render_set_scissor(VkCommandBuffer p_cmd, const std::vector<VkRect2D>& p_scissors);
    void command_bind_push_constants(const VkCommandBuffer& p_cmd, uint32_t p_size, void* r_data, uint32_t p_offset = 0);

    void command_render_draw(VkCommandBuffer p_cmd, uint32_t p_vertex_count, uint32_t p_instance_count = 1, uint32_t p_base_vertex = 0, uint32_t p_first_instance = 0);
    void command_render_draw_indexed(VkCommandBuffer p_cmd, uint32_t p_index_count, uint32_t p_instance_count = 1, uint32_t p_first_index = 0, int32_t p_vertex_offset = 0, uint32_t p_first_instance = 0);
	void command_render_draw_indexed_indirect(VkCommandBuffer p_cmd, const Buffer& p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride);
	void command_render_draw_indexed_indirect_count(VkCommandBuffer p_cmd, const Buffer& p_indirect_buffer, uint64_t p_offset, const Buffer& p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride);
	void command_render_draw_indirect(VkCommandBuffer p_cmd, const Buffer& p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride);
	void command_render_draw_indirect_count(VkCommandBuffer p_cmd, const Buffer& p_indirect_buffer, uint64_t p_offset, const Buffer& p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride);


    /*******************/
    /**** SWAPCHAIN ****/
    /*******************/

    struct Swapchain {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        ContextDriverVulkan::Surface* surface = nullptr;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        std::vector<Image> images;
        std::vector<VkFramebuffer> framebuffers;
        std::vector<VkSemaphore> present_semaphores;
        uint32_t image_index = 0;
    };

    Swapchain swapchain;

    bool _determine_swapchain_format(ContextDriverVulkan::Surface* r_surface, VkSurfaceFormatKHR &r_surface_format);
    void _swapchain_release();
    
    Error swapchain_create(ContextDriverVulkan::Surface* r_surface);
    Error swapchain_resize(uint32_t p_desired_framebuffer_count);
    void swapchain_free();
    Error swapchain_acquire_next_image(VkSemaphore p_signal_semaphore);

    /***********************/
    /**** BINDLESS HEAP ****/
    /***********************/

    struct IndexAllocator {
        uint32_t cap = 0;
        uint32_t high_water = 0;
        std::vector<uint32_t> free_list;

        uint32_t acquire() {
            if (!free_list.empty()) { uint32_t i = free_list.back(); free_list.pop_back(); return i; }
            if (high_water < cap) return high_water++;
            return UINT32_MAX;
        }
        void release(uint32_t i) { free_list.push_back(i); }
    };

    struct BindlessHeap {
        VkDescriptorSet set = VK_NULL_HANDLE;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        
        IndexAllocator sampled_alloc;
        IndexAllocator storage_alloc;
        IndexAllocator sampler_alloc;

        static constexpr uint32_t BINDING_SAMPLED = 0;
        static constexpr uint32_t BINDING_STORAGE = 1;
        static constexpr uint32_t BINDING_SAMPLER = 2;
        static constexpr uint32_t PUSH_CONSTANT_SIZE = 128;
    };

    BindlessHeap bindless_heap;

    Error bindless_heap_create(uint32_t p_sampled_count = 16384, uint32_t p_storage_count = 4096, uint32_t p_samplers_count = 256);
    void bindless_heap_free();
    uint32_t bindless_heap_alloc_sampled(VkImageView p_image_view);
    uint32_t bindless_heap_alloc_storage(VkImageView p_image_view);
    uint32_t bindless_heap_alloc_sampler(VkSampler p_sampler);
    void bindless_heap_free_sampled(uint32_t p_index);
    void bindless_heap_free_storage(uint32_t p_index);
    void bindless_heap_free_sampler(uint32_t p_index);

	/*********************/
	/**** FRAMEBUFFER ****/
	/*********************/

    VkFramebuffer framebuffer_create(VkRenderPass p_render_pass, std::vector<VkImageView>& p_image_views, VkExtent2D extent);
    void framebuffer_free(VkFramebuffer& r_framebuffer);

	/******************/
	/**** PIPELINE ****/
	/******************/
    
    // ----- SHADER -----

    enum class ShaderStage : uint8_t { Vertex, Fragment, Compute };
    
    struct ShaderCreateInfo {
        ShaderStage stage = ShaderStage::Vertex;
        const char* glsl = nullptr;
        size_t glsl_size = 0;
        const uint32_t* spirv = nullptr;
        size_t spirv_size = 0;
        const char* name = nullptr;
    };
    
    std::string shader_cache_dir;
    
    shaderc_shader_kind _shaderc_kind(ShaderStage p_stage);
    uint64_t _shader_cache_key(const ShaderCreateInfo& p_create_info, size_t p_source_len);

    VkShaderModule shader_create(const ShaderCreateInfo& p_create_info);
    void shader_free(VkShaderModule& r_shader);

    // ----- PIPELINE -----

    enum class BlendMode : uint8_t { None, Alpha, Additive, PremultipliedAlpha };

    struct GraphicsPipelineCreateInfo {
        VkShaderModule vertex_shader = VK_NULL_HANDLE;
        VkShaderModule fragment_shader = VK_NULL_HANDLE;

        VkRenderPass render_pass = VK_NULL_HANDLE;
        uint32_t subpass = 0;

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;

        bool depth_test = false;
        bool depth_write = false;
        VkCompareOp depth_compare = VK_COMPARE_OP_LESS;

        std::vector<BlendMode> blend_modes;
        std::vector<VkPipelineColorBlendAttachmentState> blend_overrides;

        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        const char* name = nullptr;
    };

    struct ComputePipelineCreateInfo {
        VkShaderModule compute_shader = VK_NULL_HANDLE;
        
        const char* name = nullptr;
    };

    struct Pipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };

    VkPipelineColorBlendAttachmentState _blend_state(BlendMode p_mode);

    Pipeline graphics_pipeline_create(const GraphicsPipelineCreateInfo& p_create_info);
    Pipeline compute_pipeline_create(const ComputePipelineCreateInfo& p_create_info);
    void pipeline_free(Pipeline& r_pipeline);
    
    void command_bind_pipeline(VkCommandBuffer p_cmd, const Pipeline& p_pipeline);
    void _command_bind_uniform_sets(VkCommandBuffer p_cmd, VkPipelineBindPoint p_bind_point, const std::vector<VkDescriptorSet>& p_sets, uint32_t p_first_set_index, uint32_t p_dynamic_offset);
    void command_bind_graphics_uniform_sets(VkCommandBuffer p_cmd, const std::vector<VkDescriptorSet>& p_sets, uint32_t p_first_set_index = 0, uint32_t p_dynamic_offset = UINT32_MAX);
    void command_bind_compute_uniform_sets(VkCommandBuffer p_cmd, const std::vector<VkDescriptorSet>& p_sets, uint32_t p_first_set_index = 0, uint32_t p_dynamic_offset = UINT32_MAX);
	void command_compute_dispatch(VkCommandBuffer p_cmd, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups);
	void command_compute_dispatch_indirect(VkCommandBuffer p_cmd, const Buffer& p_indirect_buffer, uint64_t p_offset);

    /*********************/
    /**** RENDER PASS ****/
    /*********************/

    struct RenderPassCreateInfo {
        struct Attachment {
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
            VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
            VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            bool is_depth = false;
        };
        std::vector<Attachment> attachments;
        const VkSubpassDependency* dependency = nullptr;
        const char* name = nullptr;
    };

    VkRenderPass render_pass_create(const RenderPassCreateInfo& p_create_info);
    void render_pass_free(VkRenderPass& r_render_pass);

    void command_begin_render_pass(VkCommandBuffer p_cmd, VkRenderPass p_render_pass, VkFramebuffer p_framebuffer, VkExtent2D p_extent, const std::vector<VkClearValue>& p_clear_values);
    void command_end_render_pass(VkCommandBuffer p_cmd);

	/**************/
	/**** MISC ****/
	/**************/

    void set_object_name(VkObjectType p_type, uint64_t p_handle, const char* p_name);
};

}