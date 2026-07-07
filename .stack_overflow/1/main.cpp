// Vulkan 1.3 dynamic-rendering repro: MRT (2 color + depth) hang on Intel UHD.
// Standalone. No window, no swapchain, no VMA, no pipeline/draw. Clear-only.
// Requires a Vulkan 1.3 SDK. Links vulkan-1.
//
// All three attachments are the SAME extent (1280x720). Depth is D32_SFLOAT
// (depth-only). One vkCmdBeginRendering with 2 color + 1 depth, then submit + wait.
// On Intel UHD this hangs at vkDeviceWaitIdle with no validation error.

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdio>
#include <cstdlib>

#define VK_CHECK(x) do { VkResult e_=(x); if(e_){ printf("VK_CHECK %d @line %d\n",e_,__LINE__); fflush(stdout); abort(); } } while(0)

static const uint32_t COLOR_W = 1280, COLOR_H = 720;
static const uint32_t DEPTH_W = 1280, DEPTH_H = 720; // equal to color

static VkInstance       g_instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_phys     = VK_NULL_HANDLE;
static VkDevice         g_device   = VK_NULL_HANDLE;
static uint32_t         g_family   = 0;
static VkQueue          g_queue    = VK_NULL_HANDLE;

struct Img { VkImage image; VkImageView view; VkDeviceMemory mem; };

static VKAPI_ATTR VkBool32 VKAPI_CALL dbg_cb(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* d, void*)
{ printf("[VAL] %s\n", d->pMessage); fflush(stdout); return VK_FALSE; }

static uint32_t find_mem(uint32_t bits, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(g_phys, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((bits & (1u<<i)) && (mp.memoryTypes[i].propertyFlags & props) == props) return i;
    return UINT32_MAX;
}

static Img make_image(VkFormat fmt, VkImageUsageFlags usage, VkImageAspectFlags aspect,
                      uint32_t w, uint32_t h)
{
    Img img{};
    VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = fmt;
    ici.extent = { w, h, 1 };
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK(vkCreateImage(g_device, &ici, nullptr, &img.image));

    VkMemoryRequirements req{};
    vkGetImageMemoryRequirements(g_device, img.image, &req);
    VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = find_mem(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(g_device, &mai, nullptr, &img.mem));
    VK_CHECK(vkBindImageMemory(g_device, img.image, img.mem, 0));

    VkImageViewCreateInfo vci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vci.image = img.image;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = fmt;
    vci.subresourceRange = { aspect, 0, 1, 0, 1 };
    VK_CHECK(vkCreateImageView(g_device, &vci, nullptr, &img.view));
    return img;
}

static void barrier(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
                    VkImageLayout oldL, VkImageLayout newL,
                    VkPipelineStageFlags2 srcS, VkAccessFlags2 srcA,
                    VkPipelineStageFlags2 dstS, VkAccessFlags2 dstA)
{
    VkImageMemoryBarrier2 b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    b.srcStageMask = srcS; b.srcAccessMask = srcA;
    b.dstStageMask = dstS; b.dstAccessMask = dstA;
    b.oldLayout = oldL;    b.newLayout = newL;
    b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image;
    b.subresourceRange = { aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &b;
    vkCmdPipelineBarrier2(cmd, &dep);
}

int main()
{
    // --- instance with validation ---
    VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app.apiVersion = VK_API_VERSION_1_3;
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    const char* iexts[]  = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    VkInstanceCreateInfo ici{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    ici.pApplicationInfo = &app;
    ici.enabledLayerCount = 1;     ici.ppEnabledLayerNames = layers;
    ici.enabledExtensionCount = 1; ici.ppEnabledExtensionNames = iexts;
    VK_CHECK(vkCreateInstance(&ici, nullptr, &g_instance));

    VkDebugUtilsMessengerCreateInfoEXT dci{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    dci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    dci.pfnUserCallback = dbg_cb;
    auto mk = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT msgr = VK_NULL_HANDLE; if (mk) mk(g_instance, &dci, nullptr, &msgr);

    // --- device (dynamic rendering + sync2) ---
    uint32_t n = 0; vkEnumeratePhysicalDevices(g_instance, &n, nullptr);
    std::vector<VkPhysicalDevice> devs(n); vkEnumeratePhysicalDevices(g_instance, &n, devs.data());
    g_phys = devs[0];
    VkPhysicalDeviceProperties props{}; vkGetPhysicalDeviceProperties(g_phys, &props);
    printf("Device: %s\n", props.deviceName); fflush(stdout);

    uint32_t qn = 0; vkGetPhysicalDeviceQueueFamilyProperties(g_phys, &qn, nullptr);
    std::vector<VkQueueFamilyProperties> qs(qn); vkGetPhysicalDeviceQueueFamilyProperties(g_phys, &qn, qs.data());
    for (uint32_t i = 0; i < qn; i++) if (qs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { g_family = i; break; }

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    qci.queueFamilyIndex = g_family; qci.queueCount = 1; qci.pQueuePriorities = &prio;

    VkPhysicalDeviceVulkan13Features f13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    f13.dynamicRendering = VK_TRUE; f13.synchronization2 = VK_TRUE;
    VkPhysicalDeviceFeatures2 f2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    f2.pNext = &f13;

    VkDeviceCreateInfo dci2{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dci2.pNext = &f2;
    dci2.queueCreateInfoCount = 1; dci2.pQueueCreateInfos = &qci;
    VK_CHECK(vkCreateDevice(g_phys, &dci2, nullptr, &g_device));
    vkGetDeviceQueue(g_device, g_family, 0, &g_queue);

    // --- attachments: two color + one depth, ALL 1280x720 ---
    Img color0 = make_image(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, COLOR_W, COLOR_H);
    Img color1 = make_image(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, COLOR_W, COLOR_H);
    Img depth  = make_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, DEPTH_W, DEPTH_H);
    printf("color=%ux%u depth=%ux%u\n", COLOR_W, COLOR_H, DEPTH_W, DEPTH_H); fflush(stdout);

    // --- command buffer ---
    VkCommandPoolCreateInfo pci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    pci.queueFamilyIndex = g_family;
    VkCommandPool pool; VK_CHECK(vkCreateCommandPool(g_device, &pci, nullptr, &pool));
    VkCommandBufferAllocateInfo cbi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbi.commandPool = pool; cbi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbi.commandBufferCount = 1;
    VkCommandBuffer cmd; VK_CHECK(vkAllocateCommandBuffers(g_device, &cbi, &cmd));

    VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));

    barrier(cmd, color0.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    barrier(cmd, color1.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    barrier(cmd, depth.image, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    VkRenderingAttachmentInfo c0{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    c0.imageView = color0.view; c0.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    c0.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; c0.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    c0.clearValue.color = { { 0,0,1,1 } };
    VkRenderingAttachmentInfo c1 = c0; c1.imageView = color1.view;

    VkRenderingAttachmentInfo d{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    d.imageView = depth.view; d.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    d.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; d.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    d.clearValue.depthStencil = { 0.0f, 0 };

    VkRenderingAttachmentInfo cols[2] = { c0, c1 };
    VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    ri.renderArea = { { 0,0 }, { COLOR_W, COLOR_H } };
    ri.layerCount = 1;
    ri.colorAttachmentCount = 2;
    ri.pColorAttachments = cols;
    ri.pDepthAttachment = &d;

    printf("before vkCmdBeginRendering\n"); fflush(stdout);
    vkCmdBeginRendering(cmd, &ri);
    vkCmdEndRendering(cmd);
    printf("after vkCmdEndRendering\n"); fflush(stdout);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    VK_CHECK(vkQueueSubmit(g_queue, 1, &si, VK_NULL_HANDLE));
    printf("submitted, waiting...\n"); fflush(stdout);
    VK_CHECK(vkDeviceWaitIdle(g_device)); // hang surfaces here on Intel
    printf("done\n"); fflush(stdout);

    while(true) {}
    return 0;
}