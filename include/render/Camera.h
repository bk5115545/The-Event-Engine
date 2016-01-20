#pragma once

#include "util/Definitions.h"

class Camera {
  public:
    virtual bool initialize() = 0;
    virtual void update(std::shared_ptr<void>) = 0;
};
