#pragma once

#pragma comment(lib, "dwmapi.lib")

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vk_mem_alloc.h>

#include <shaderc/shaderc.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <stb_image.h>

#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <array>
#include <set>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cstdio>

#include "Core/Log.h"
#include "Core/Assert.h"
