#pragma once

#include <vector>
#include <memory>

#include "util/definitions.h"

class Renderer {
public:
    // virtual ~Renderer() = 0;

    virtual bool Initialize() = 0;

    virtual void PreDraw() = 0;
    virtual void Draw() = 0;
    virtual void PostDraw() = 0;

    virtual uint32 width() = 0;
    virtual uint32 height() = 0;
};
