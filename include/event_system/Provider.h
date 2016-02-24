#pragma once

#include <memory>

class Provider {
  public:
    virtual std::string get_name() { return std::string(""); };
    virtual std::shared_ptr<void> provide(std::shared_ptr<void> arg) = 0;
    virtual std::shared_ptr<void> get_parameter_data() = 0; // need a MUCH better way to do this
};
