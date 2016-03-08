#pragma once

#include <functional>
#include <memory>
#include <iostream>

#include "event_system/Dispatcher.h"

// These macros might need to be updated if the SubscriptionFunction spec is changed
#define Function_Cast(func_ref) (SubscriptionFunction*) func_ref

typedef void(SubscriptionFunction(void*, std::shared_ptr<void>));
typedef void(CallTypeFunction(std::shared_ptr<void>));

class Subscriber {
  public:
    void* _owner;
    bool _serialized = true;

    Subscriber(void* owner, SubscriptionFunction* func, bool serialized = true) {
        this->_owner = owner;
        this->_serialized = serialized;
        this->method = func;

        call = std::bind(&Subscriber::_threadsafe_call, this, std::placeholders::_1);
    }

    Subscriber(Subscriber& other) {
        this->_owner = other._owner;
        this->method = other.method;
        this->_serialized = other._serialized;

        call = std::bind(&Subscriber::_threadsafe_call, this, std::placeholders::_1);
    }

    Subscriber(Subscriber&& other) {
        this->_owner = other._owner;
        this->method = other.method;
        this->_serialized = other._serialized;

        call = std::bind(&Subscriber::_threadsafe_call, this, std::placeholders::_1);
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

    std::function<CallTypeFunction> call;

  protected:
    std::function<SubscriptionFunction> method;
    std::mutex concurrent_call_block;

    // This method needs to be updated if the SubscriptionFunction specification is changed
    void _threadsafe_call(std::shared_ptr<void> arg) {
        std::lock_guard<std::mutex> lock(concurrent_call_block);
        // std::cout << "_call1 " << arg << std::endl;
        method(_owner, arg);
        // std::cout << "_call2 " << arg << std::endl;
    }

    void _std_call(std::shared_ptr<void> arg) { method(_owner, arg); }
};
