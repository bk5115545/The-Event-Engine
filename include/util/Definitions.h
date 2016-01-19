#pragma once

#define UNUSED(x) (void)x;

// Unsigned Integers
typedef unsigned long long int uint64;
typedef unsigned int uint32;
typedef unsigned short int uint16;

// Signed Integers
typedef long long int int64;
typedef int int32;
typedef short int int16;

// Floats
typedef float float32;
typedef double float64;

// Event Types
enum GameEvent { kGameNA, kGameUp, kGameDown, kGameLeft, kGameRight, kGameW, kGameA, kGameS, kGameD, kGameSpace };
enum ActorEvent { kMoveLeft, kMoveRight, kMoveUp, kMoveDown, kTurnLeft, kTurnRight };
enum State { kFacingLeft, kFacingRight, kStationary, kJumping, kStopped };

// Constants
const float32 PI = 3.14159f;



// Includes for EventSystem (should restructure to not need these globally.
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <utility>

#include "event_system/Subscriber.h"
