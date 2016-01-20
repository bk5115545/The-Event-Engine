#pragma once

/**
 * Macro to tell linter/compiler that we know an argument/variable is unused and to move on with our lives.
 **/
#define UNUSED(x) (void) x;

#include <thread>
#include <chrono>

/**
 * Shorthand method to access cross-platform "sleep" functionality.
 * Will cause compile error without explicit inline or if inline is not respected.
 **/
inline void sleep(int time_millis) { std::this_thread::sleep_for(std::chrono::milliseconds(time_millis)); }
