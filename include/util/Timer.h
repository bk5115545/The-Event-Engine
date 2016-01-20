#pragma once

#include "util/Definitions.h"

class Timer {
  private:
    std::chrono::high_resolution_clock::time_point start_ticks;

    float delta_time;

  public:
    Timer();

    void start();
    void reset();

    bool isRunning();

    void update();

    float get_deltaTime();
};
