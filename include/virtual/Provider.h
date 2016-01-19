#include <memory>

#inclue "util/Definitions.h"

template<typename T, ...Args>
class Provider {
    virtual T provide(Args) = 0;
};
