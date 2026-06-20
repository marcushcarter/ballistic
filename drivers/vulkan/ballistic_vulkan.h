// #pragma once

// // IWYU pragma: begin_exports.
// #ifdef USE_VOLK
// #include <volk.h>
// #else
// #include <cstdint>
// #define VK_NO_STDINT_H
// #include <vulkan/vulkan.h>
// #ifdef VK_USE_PLATFORM_XLIB_KHR
// #undef CursorShape
// #endif
// #endif