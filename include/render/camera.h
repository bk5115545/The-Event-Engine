#pragma once

#include "util/definitions.h"

class Camera {
 public:
    virtual bool Initialize() = 0;
    virtual void Update(std::shared_ptr<void>) = 0;
};
