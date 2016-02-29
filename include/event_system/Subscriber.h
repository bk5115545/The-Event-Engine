#pragma once

#include <functional>
#include <memory>
#include <iostream>

#include "event_system/Dispatcher.h"

// These macros might need to be updated if the SubscriptionFunction spec is changed
#define Function_Cast(func_ref) (void*) func_ref
#define Function_UnCast(void_p) std::function<SubscriptionFunction>(*(SubscriptionFunction*)(void_p))

typedef void(SubscriptionFunction(std::shared_ptr<void>));

class Subscriber {
  public:
    bool serialized = true;

    Subscriber(void* owner, void* func, bool serialized = true) {
        this->owner = owner;
        this->serialized = serialized;
        this->method = Function_UnCast(func);

        call = std::bind(&Subscriber::_call, this, std::placeholders::_1);
    }
    Subscriber(Subscriber& other) {
        this->owner = other.owner;
        this->method = other.method;
        this->serialized = other.serialized;

        call = std::bind(&Subscriber::_call, this, std::placeholders::_1);
    }

    Subscriber(Subscriber&& other) {
        this->owner = other.owner;
        this->method = other.method;
        this->serialized = other.serialized;

        call = std::bind(&Subscriber::_call, this, std::placeholders::_1);
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

    std::function<SubscriptionFunction> call;

    void* owner;

  protected:
    std::function<SubscriptionFunction> method;
    std::mutex concurrent_call_block;

    // This method needs to be updated if the SubscriptionFunction specification is changed
    void _call(std::shared_ptr<void> arg) {
        std::lock_guard<std::mutex> lock(concurrent_call_block);
        std::cout << "_call1 " << arg << std::endl;
        method(arg);
        std::cout << "_call2 " << arg << std::endl;
    }
};
