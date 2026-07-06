#pragma once
#include <cstdint>

namespace ballistic {

struct EditorContext;

struct ViewportPanel {
    bool open = false;
    void draw(EditorContext& ctx);
};

}