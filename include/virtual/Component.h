#pragma once

#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

class Component {
  public:
    virtual void process(std::shared_ptr<void> event_data) = 0;
};
