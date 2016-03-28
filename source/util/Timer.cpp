#include "util/Timer.h"

#include <chrono>

Timer::Timer() { delta_time = (float)0; }

void Timer::start() { start_ticks = std::chrono::high_resolution_clock::now(); }

void Timer::reset() { start_ticks = std::chrono::high_resolution_clock::now(); }

void Timer::update() {
    delta_time = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double>>(
                                        std::chrono::high_resolution_clock::now() - start_ticks)
                                        .count());
    start_ticks = std::chrono::high_resolution_clock::now();
}

float32 Timer::get_deltaTime() { return delta_time; }
