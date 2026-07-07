# Vulkan dynamic rendering (vkCmdBeginRendering) hangs on Intel UHD when using a depth attachment and 1 or more color attachments

This is my second attempt at asking about this issue. I'm building a render graph for an in-house Vulkan engine using dynamic rendering (Vulkan 1.3, `dynamicRendering` + `synchronization2`), on Win32. I've hit a case where a single `vkCmdBeginRendering` pass that has both a depth attachment and one or more color attachments hangs the program with no crash message and no validation error. Depth-only (1 depth, 0 color) works. Color-only (1+ color, 0 depth) works. Mixing depth with any color attachment hangs. All attachments share identical extent and sample count; depth is `VK_FORMAT_D32_SFLOAT` (depth-only, so this is not a stencil-aspect layout issue).

The hang comes up at `vkbeginRenderingInfo` of the pass that has the swapchain image view.

`VK_LAYER_KHRONOS_validation` is enabled and prints nothing. No VUID and no warning; the device just hangs. Swapping the same program to a `VkRenderPass` + `VkFramebuffer`, with identical images, formats, and barriers, works perfectly, which is why I think it could be a driver-side issue rather than my code, but I want confirmation.

The repro below is standalone. It creates two `R8G8B8A8_UNORM` color targets and one `D32_SFLOAT` depth target at the same extent, barriers them, does one pass with the 2 color + depth and then one with the swapcahin (`vkCmdBeginRendering`/`vkCmdEndRendering`), submits, and then keeps looping. It requires a Vulkan 1.3 SDK. If it runs to completion on a newer or discrete GPU, then it is most likely an Intel integrated driver bug.

Environment: Intel UHD Graphics (integrated), Windows 11, Vulkan SDK 1.4.350.0, instance apiVersion 1.3. Driver version 31.0.101.2127.

__Question: is this a validation-layer gap, an Intel driver bug in the dynamic-rendering color+depth path, or defined behavior I'm not understanding?__

```cmake
# CMakeLists.txt

cmake_minimum_required(VERSION 3.28)
project(MRTRepro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(mrt_repro main.cpp)

target_link_libraries(mrt_repro PRIVATE Vulkan::Vulkan)

target_include_directories(mrt_repro PRIVATE
    "$ENV{VULKAN_SDK}/Include"
    "$ENV{VULKAN_SDK}/Include/vma"
)

if (MSVC)
    target_compile_options(mrt_repro PRIVATE /W3)
endif()
```

```c++
// main.cpp

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

#define VK_CHECK(x) do { VkResult e_=(x); if(e_!=VK_SUCCESS){ printf("VK_CHECK %d at %s:%d\n",e_,__FILE__,__LINE__); fflush(stdout); abort(); } } while(0)
#define LOG(...) do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while(0)

static const uint32_t WIDTH  = 1280;
static const uint32_t HEIGHT = 720;
static const uint32_t FRAMES = 3;

static VkInstance g_instance = VK_NULL_HANDLE;
static VkDebugUtilsMessengerEXT g_messenger = VK_NULL_HANDLE;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;
static VkPhysicalDevice g_phys = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static uint32_t g_gfx_family = 0;
static VkQueue g_queue = VK_NULL_HANDLE;
static VmaAllocator g_allocator = nullptr;

static VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
static VkFormat g_swapchain_format = VK_FORMAT_UNDEFINED;
static VkExtent2D g_swapchain_extent = {};
static std::vector<VkImage> g_swapchain_images;
static std::vector<VkImageView> g_swapchain_views;

struct Img { VkImage image; VkImageView view; VmaAllocation alloc; VkExtent3D extent; };
static Img g_color0;
static Img g_color1;
static Img g_depth;

static std::vector<VkCommandPool> g_pools;
static std::vector<VkCommandBuffer> g_cmds;
static std::vector<VkSemaphore> g_acquire_sems;
static std::vector<VkSemaphore> g_render_sems;
static std::vector<VkFence> g_fences;

static bool g_running = true;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT sev, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    const char* lvl = (sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : (sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN" : "INFO";
    LOG("[VULKAN][%s] %s", lvl, data->pMessage);
    return VK_FALSE;
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CLOSE || msg == WM_DESTROY) { g_running = false; return 0; }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void image_barrier(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access, VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access)
{
    VkImageMemoryBarrier2 b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    b.srcStageMask = src_stage;  b.srcAccessMask = src_access;
    b.dstStageMask = dst_stage;  b.dstAccessMask = dst_access;
    b.oldLayout = old_layout;    b.newLayout = new_layout;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image;
    b.subresourceRange = { aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };

    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &b;
    vkCmdPipelineBarrier2(cmd, &dep);
}

static Img make_image(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, uint32_t w, uint32_t h, const char* tag)
{
    Img img;
    img.extent = { w, h, 1 };

    VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = format;
    ici.extent = { w, h, 1 };
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VK_CHECK(vmaCreateImage(g_allocator, &ici, &aci, &img.image, &img.alloc, nullptr));

    VkImageViewCreateInfo vci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vci.image = img.image;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = format;
    vci.subresourceRange = { aspect, 0, 1, 0, 1 };
    VK_CHECK(vkCreateImageView(g_device, &vci, nullptr, &img.view));

    LOG("MAKE IMAGE %s: image=%p view=%p ext=%ux%u", tag, (void*)img.image, (void*)img.view, w, h);
    return img;
}

static void create_instance()
{
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
    LOG("---- AVAILABLE INSTANCE LAYERS (%u) ----", layer_count);
    for (auto& l : layers) LOG("  %s", l.layerName);
    LOG("----------------------------------------");

    VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app.pApplicationName = "MRTRepro";
    app.apiVersion = VK_API_VERSION_1_3;

    const char* exts[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };
    const char* val_layers[] = { "VK_LAYER_KHRONOS_validation" };

    VkDebugUtilsMessengerCreateInfoEXT dbg{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg.pfnUserCallback = debug_cb;

    VkInstanceCreateInfo ci{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    ci.pNext = &dbg;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = 3;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledLayerCount = 1;
    ci.ppEnabledLayerNames = val_layers;

    VK_CHECK(vkCreateInstance(&ci, nullptr, &g_instance));

    auto create_msg = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT");
    if (create_msg) create_msg(g_instance, &dbg, nullptr, &g_messenger);
}

static void create_device()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(g_instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devs(count);
    vkEnumeratePhysicalDevices(g_instance, &count, devs.data());
    g_phys = devs[0];

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(g_phys, &props);
    LOG("Using device: %s (api %u.%u.%u)", props.deviceName, VK_API_VERSION_MAJOR(props.apiVersion), VK_API_VERSION_MINOR(props.apiVersion), VK_API_VERSION_PATCH(props.apiVersion));

    uint32_t qcount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_phys, &qcount, nullptr);
    std::vector<VkQueueFamilyProperties> qprops(qcount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_phys, &qcount, qprops.data());
    for (uint32_t i = 0; i < qcount; i++) {
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(g_phys, i, g_surface, &present);
        if ((qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { g_gfx_family = i; break; }
    }

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    qci.queueFamilyIndex = g_gfx_family;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    VkPhysicalDeviceVulkan13Features f13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    f13.dynamicRendering = VK_TRUE;
    f13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features f12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    f12.bufferDeviceAddress = VK_TRUE;
    f12.pNext = &f13;

    VkPhysicalDeviceFeatures2 f2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    f2.pNext = &f12;
    vkGetPhysicalDeviceFeatures2(g_phys, &f2);

    const char* dev_exts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dci.pNext = &f2;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = dev_exts;

    VK_CHECK(vkCreateDevice(g_phys, &dci, nullptr, &g_device));
    vkGetDeviceQueue(g_device, g_gfx_family, 0, &g_queue);

    VmaAllocatorCreateInfo aci{};
    aci.physicalDevice = g_phys;
    aci.device = g_device;
    aci.instance = g_instance;
    aci.vulkanApiVersion = VK_API_VERSION_1_3;
    aci.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
    VK_CHECK(vmaCreateAllocator(&aci, &g_allocator));
}

static void create_swapchain()
{
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_phys, g_surface, &caps);
    g_swapchain_extent = caps.currentExtent;

    uint32_t fmt_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys, g_surface, &fmt_count, nullptr);
    std::vector<VkSurfaceFormatKHR> fmts(fmt_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys, g_surface, &fmt_count, fmts.data());
    VkSurfaceFormatKHR chosen = fmts[0];
    for (auto& f : fmts) if (f.format == VK_FORMAT_B8G8R8A8_UNORM) { chosen = f; break; }
    g_swapchain_format = chosen.format;

    uint32_t image_count = caps.minImageCount < FRAMES ? FRAMES : caps.minImageCount;
    if (caps.maxImageCount && image_count > caps.maxImageCount) image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    sci.surface = g_surface;
    sci.minImageCount = image_count;
    sci.imageFormat = chosen.format;
    sci.imageColorSpace = chosen.colorSpace;
    sci.imageExtent = g_swapchain_extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(g_device, &sci, nullptr, &g_swapchain));

    uint32_t sc_count = 0;
    vkGetSwapchainImagesKHR(g_device, g_swapchain, &sc_count, nullptr);
    g_swapchain_images.resize(sc_count);
    vkGetSwapchainImagesKHR(g_device, g_swapchain, &sc_count, g_swapchain_images.data());

    g_swapchain_views.resize(sc_count);
    for (uint32_t i = 0; i < sc_count; i++) {
        VkImageViewCreateInfo vci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vci.image = g_swapchain_images[i];
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = g_swapchain_format;
        vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        VK_CHECK(vkCreateImageView(g_device, &vci, nullptr, &g_swapchain_views[i]));
    }

    LOG("SWAPCHAIN: %ux%u images=%u format=%d", g_swapchain_extent.width, g_swapchain_extent.height, sc_count, g_swapchain_format);
}

static void create_frame_resources()
{
    g_pools.resize(FRAMES);
    g_cmds.resize(FRAMES);
    g_acquire_sems.resize(FRAMES);
    g_render_sems.resize(FRAMES);
    g_fences.resize(FRAMES);

    for (uint32_t i = 0; i < FRAMES; i++) {
        VkCommandPoolCreateInfo pci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pci.queueFamilyIndex = g_gfx_family;
        VK_CHECK(vkCreateCommandPool(g_device, &pci, nullptr, &g_pools[i]));

        VkCommandBufferAllocateInfo cbi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cbi.commandPool = g_pools[i];
        cbi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbi.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(g_device, &cbi, &g_cmds[i]));

        VkSemaphoreCreateInfo semi{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VK_CHECK(vkCreateSemaphore(g_device, &semi, nullptr, &g_acquire_sems[i]));
        VK_CHECK(vkCreateSemaphore(g_device, &semi, nullptr, &g_render_sems[i]));

        VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(g_device, &fci, nullptr, &g_fences[i]));
    }
}

static void render_frame(uint32_t frame)
{
    VK_CHECK(vkWaitForFences(g_device, 1, &g_fences[frame], VK_TRUE, UINT64_MAX));

    uint32_t image_index = 0;
    VkResult acq = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, g_acquire_sems[frame], VK_NULL_HANDLE, &image_index);
    if (acq == VK_ERROR_OUT_OF_DATE_KHR) return;

    VK_CHECK(vkResetFences(g_device, 1, &g_fences[frame]));
    VK_CHECK(vkResetCommandPool(g_device, g_pools[frame], 0));

    VkCommandBuffer cmd = g_cmds[frame];
    VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));

    image_barrier(cmd, g_color0.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    image_barrier(cmd, g_color1.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    image_barrier(cmd, g_depth.image, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    {
        VkRenderingAttachmentInfo c0{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        c0.imageView = g_color0.view;
        c0.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        c0.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        c0.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        c0.clearValue.color = { { 0.0f, 0.0f, 1.0f, 1.0f } };

        VkRenderingAttachmentInfo c1 = c0;
        c1.imageView = g_color1.view;

        VkRenderingAttachmentInfo d{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        d.imageView = g_depth.view;
        d.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        d.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        d.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        d.clearValue.depthStencil = { 0.0f, 0 };

        VkRenderingAttachmentInfo cols[2] = { c0, c1 };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, { g_color0.extent.width, g_color0.extent.height } };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 2;
        ri.pColorAttachments = cols;
        ri.pDepthAttachment = &d;

        vkCmdBeginRendering(cmd, &ri);
        vkCmdEndRendering(cmd);
    }

    image_barrier(cmd, g_swapchain_images[image_index], VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    {
        VkRenderingAttachmentInfo c{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        c.imageView = g_swapchain_views[image_index];
        c.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        c.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        c.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        c.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea = { { 0, 0 }, g_swapchain_extent };
        ri.layerCount = 1;
        ri.colorAttachmentCount = 1;
        ri.pColorAttachments = &c;

        vkCmdBeginRendering(cmd, &ri);
        vkCmdEndRendering(cmd);
    }

    image_barrier(cmd, g_swapchain_images[image_index], VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &g_acquire_sems[frame];
    si.pWaitDstStageMask = &wait_stage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &g_render_sems[frame];
    VK_CHECK(vkQueueSubmit(g_queue, 1, &si, g_fences[frame]));

    VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &g_render_sems[frame];
    pi.swapchainCount = 1;
    pi.pSwapchains = &g_swapchain;
    pi.pImageIndices = &image_index;
    vkQueuePresentKHR(g_queue, &pi);
}

int main()
{
    HINSTANCE hinst = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hinst;
    wc.lpszClassName = L"MRTReproClass";
    RegisterClassExW(&wc);

    RECT rect = { 0, 0, (LONG)WIDTH, (LONG)HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"MRT Repro", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hinst, nullptr);

    create_instance();

    VkWin32SurfaceCreateInfoKHR sci{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    sci.hinstance = hinst;
    sci.hwnd = hwnd;
    VK_CHECK(vkCreateWin32SurfaceKHR(g_instance, &sci, nullptr, &g_surface));

    create_device();
    create_swapchain();
    create_frame_resources();

    uint32_t w = g_swapchain_extent.width;
    uint32_t h = g_swapchain_extent.height;
    VkImageUsageFlags color_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageUsageFlags depth_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    g_color0 = make_image(VK_FORMAT_R8G8B8A8_UNORM, color_usage, VK_IMAGE_ASPECT_COLOR_BIT, w, h, "color0");
    g_color1 = make_image(VK_FORMAT_R8G8B8A8_UNORM, color_usage, VK_IMAGE_ASPECT_COLOR_BIT, w, h, "color1");
    g_depth = make_image(VK_FORMAT_D32_SFLOAT, depth_usage, VK_IMAGE_ASPECT_DEPTH_BIT, w, h, "depth");

    uint32_t frame = 0;
    MSG msg{};
    while (g_running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!g_running) break;
        render_frame(frame);
        frame = (frame + 1) % FRAMES;
    }

    vkDeviceWaitIdle(g_device);
    return 0;
}
```