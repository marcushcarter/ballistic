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
    ContextDriverVulkan* context_driver = nullptr;
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

    Error initialize(ContextDriverVulkan& r_context_driver, uint32_t p_device_index, uint32_t p_frame_count);
    void shutdown();

    Error device_wait_idle();

    /****************/
    /**** MEMORY ****/
    /****************/

    VmaAllocator allocator = nullptr;

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

    /******************/
    /**** COMMANDS ****/
    /******************/
    
    // ----- POOL -----

    struct CommandPool {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    };

    CommandPool command_pool_create(uint32_t p_queue_family_index, VkCommandBufferLevel p_buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void command_pool_free(CommandPool& r_cmd_pool);
    Error command_pool_reset(CommandPool& r_cmd_pool);
    
    // ----- BUFFER -----

    VkCommandBuffer command_buffer_create(CommandPool& p_cmd_pool);
    Error command_buffer_begin(VkCommandBuffer p_cmd_buffer, VkCommandBufferUsageFlags p_flags = 0);
    Error command_buffer_end(VkCommandBuffer p_cmd_buffer);
    Error swapchain_update();
    
	/****************/
	/**** IMAGES ****/
	/****************/

    struct BarrierState {
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
        BarrierState state;

        VkExtent3D extent = {};
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t mip_levels = 1, layers = 1;
        VkMemoryRequirements mem_req = {};
    };

    Image image_create(const ImageCreateInfo& p_create_info, VkExtent3D p_extent);
    Image image_create_dedicated(const ImageCreateInfo& p_create_info, VkExtent3D p_extent);
    Error image_bind(Image& r_image, VmaAllocation p_allocation);
    Error image_create_view(Image& r_image);
    void image_free(Image& r_image);

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

    VkSampler sampler_create(const SamplerCreateInfo& p_create_info);
    void sampler_free(VkSampler& r_sampler);

    /*******************/
    /**** SWAPCHAIN ****/
    /*******************/

    struct Swapchain {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        ContextDriverVulkan::Surface* surface = nullptr;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        std::vector<Image> images;
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

	/*********************/
	/**** DESCRIPTORS ****/
	/*********************/

    // ----- BINDLESS HEAP -----

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
    void bindless_heap_free_sampled(uint32_t p_index);
    void bindless_heap_free_storage(uint32_t p_index);
    void bindless_heap_register_sampler(uint32_t p_index, VkSampler p_sampler);

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

	/*********************/
	/**** FRAMEBUFFER ****/
	/*********************/

    VkFramebuffer framebuffer_create(VkRenderPass p_render_pass, std::vector<VkImageView>& p_image_views, VkExtent2D extent);
    void framebuffer_free(VkFramebuffer& r_framebuffer);

	/****************/
	/**** SHADER ****/
	/****************/

    std::string shader_cache_dir;

    enum class ShaderStage : uint8_t { Vertex, Fragment, Compute };
    
    struct ShaderCreateInfo {
        ShaderStage stage = ShaderStage::Vertex;
        const char* glsl_source = nullptr;
        const uint32_t* spirv = nullptr;
        size_t spirv_size = 0;
        const char* name = nullptr;
    };

    VkShaderModule shader_create(const ShaderCreateInfo& p_create_info);
    void shader_free(VkShaderModule& r_shader);

	/******************/
	/**** PIPELINE ****/
	/******************/

    // ----- CACHE -----

    // ----- PIPELINE -----

    // struct Pipeline {
    //     VkPipeline pipeline = VK_NULL_HANDLE;
    //     VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // };

    // Pipeline graphics_pipeline_create(const GraphicsPipelineCreateInfo& p_create_info);
    // Pipeline compute_pipeline_create(const ComputePipelineCreateInfo& p_create_info);
    // void pipeline_destroy(Pipeline& r_pipeline);

	/******************/
	/**** COMMANDS ****/
	/******************/

    // void command_render_set_viewport(VkCommandBuffer p_cmd_buffer, VectorView<Rect2i> p_viewports);
	// void command_render_set_scissor(VkCommandBuffer p_cmd_buffer, VectorView<Rect2i> p_scissors);

    // bind vertex buffers
    // bind index buffers

    // draw indirect
    // draw
    // draw indexed

    // bid render/compute pipeline
    // bind render/comput uniform sets

	/***************/
	/**** UTILS ****/
	/***************/

    void set_object_name(VkObjectType p_type, uint64_t p_handle, const char* p_name);

};

}