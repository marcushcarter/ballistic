#pragma once

namespace ballistic {
struct Renderer;
namespace drivers { struct ImGuiDriver; }

struct DevContext
{
    Renderer* renderer = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
};

}