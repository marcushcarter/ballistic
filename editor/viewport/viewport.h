#pragma once
#include <editor/panel.h>

namespace ballistic {

struct Viewport : Panel {
    const char* name() const override { return "Viewport"; }
    void draw(EditorContext& ctx) override;
};

}