#pragma once

#include <functional>
#include <memory>

#include "util/definitions.h"
#include "event_system/Dispatcher.h"

typedef void (SubscriptionFunction(std::shared_ptr<void>));

class Subscriber {
    public:
        bool serialized = true;

        Subscriber(void* owner, bool serialized = true) {
            this->owner = owner;
            this->serialized = serialized;
        }
        Subscriber(Subscriber &other) {
            this->owner = other.owner;
            this->method = other.method;
            this->serialized = other.serialized;
        }

        Subscriber(Subscriber &&other) {
            this->owner = other.owner;
            this->method = other.method;
            this->serialized = other.serialized;
        }

        // Returns strongly typed std::bind objects with typed args and returns
        // Need a way to store this so that Dispatcher can call it
/*
        template<typename R, typename C, typename... Args>
        std::function<R(Args...)> bind(R (C::* func)(Args...), C& instance) {
            return [=](Args... args){ return (instance.*func)(args...); };
        }

        template<typename R, typename C, typename... Args>
        std::function<R(Args...)> bind(R (C::* func)(Args...) const, C const& instance) {
            return [=](Args... args){ return (instance.*func)(args...); };
        }
*/
        std::function<SubscriptionFunction> method;

        void* owner;
};
