#pragma once

#include "Utility/GUID.h"

#include "Root/Singleton.h"
#include "Root/Root.h"
#include "Root/LogManager/LogManager.h"
#include "Root/LogManager/Log.h"

#include "Core/IApplication.h"
#include "Core/Layers/IEvent.h"
#include "Core/Layers/Events.h"
#include "Core/Layers/ILayer.h"
#include "Core/Layers/Layerstack.h"
#include "Core/Window/WindowInfo.h"
#include "Core/Window/Window.h"

#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/RenderDevice/IRenderDevice.h"
#include "Renderer/RenderDevice/GLRenderDevice.h"
#include "Renderer/Backends/OpenGL/all.h"
#include "Renderer/Backends/OpenGL/ShaderGLM.h"

#include "Scene/Components/Components.h"
#include "Scene/EntityHandle.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

// #include <stb_image.h>

// #include <imgui.h>
// #include <imgui_internal.h>
// #include <imgui_impl_opengl3.h>
// #include <imgui_impl_glfw.h>

// #include <nlohmann/json.hpp>

// #include <tinyfiledialogs.h>

#include "bepch.h"