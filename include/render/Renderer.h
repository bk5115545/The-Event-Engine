#pragma once

#include <vector>
#include <memory>

#include "render/Camera.h"

class Renderer {
  public:
    virtual bool initialize() = 0;

    virtual void preDraw() = 0;
    virtual void draw() = 0;
    virtual void postDraw() = 0;

    virtual int get_width() = 0;
    virtual int get_height() = 0;

    virtual void set_camera(Camera* cam) = 0;
};
