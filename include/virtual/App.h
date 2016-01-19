#pragma once

#include "render/Renderer.h"

class App {
protected:

public:
    virtual bool initialize(std::shared_ptr<Renderer> renderer) = 0;
    virtual void reset() = 0;
    virtual bool loadLevel(std::string config_file_name) = 0;
    virtual void run() = 0;
    virtual void update(float delta_time) = 0;
};
