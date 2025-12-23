#pragma once

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <shellapi.h>
#endif

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <entt/entt.hpp>

#include <nlohmann/json.hpp>

#include <tinyfiledialogs.h>

#include "Core/Utils.h"