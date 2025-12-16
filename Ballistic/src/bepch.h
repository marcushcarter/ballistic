#pragma once

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#endif

#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include <set>
#include <cstdint>
#include <algorithm>
#include <filesystem>

#include <glad/glad.h>
#include <glballistic/all.h>
#include <GLFW/glfw3.h>

#include <entt/entt.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>