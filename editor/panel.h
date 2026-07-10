#pragma once

namespace ballistic {

struct EditorContext;

struct Panel {
    bool open = true;

    virtual ~Panel() = default;
    virtual const char* name() const = 0;
    virtual void draw(EditorContext& ctx) = 0;
};

}