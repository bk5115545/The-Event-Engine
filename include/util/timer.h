#pragma once

#include "util/definitions.h"

class Timer {
private:
    // The clock time when the timer started
    std::chrono::high_resolution_clock::time_point start_ticks_;

    // Time Delta!
    float32 delta_time_;

public:
    // Initializes variables
    Timer();

    // The various clock actions
    void Start();
    void Reset();

    // Checks the status of the timer
    bool IsRunning();

    // Update DeltaTime
    void Update();

    // Returns the time in ms from the last call
    float32 DeltaTime();
};
