#pragma once

namespace ballistic {

struct Renderer;
struct DevTools;

struct EditorContext
{
    Renderer* renderer = nullptr;
    DevTools* dev_tools = nullptr;
};

}