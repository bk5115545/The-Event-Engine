#pragma once

#include <map>
#include <memory>

// TODO(bk5115545) convert this to use std::shared_mutex
// for (concurrent read/exclusive write) when c++1z is common (don't pull in boost)
#include <mutex>
#include <thread>
#include <utility>

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"
#include "virtual/Provider.h"

class ProviderRegistry {
  private:
    bool inited = false;
    ProviderRegistry* theInstance;
    std::map<EventType, std::shared_ptr<Provider>>* provider_lookup;

    std::mutex lookup_mutex;

    ProviderRegistry();
    ProviderRegistry(const ProviderRegistry&);            // disallow copying
    ProviderRegistry& operator=(const ProviderRegistry&); // disallow copying

  public:
    ProviderRegistry* GetInstance();

    bool hasProvider(EventType key);
    bool provide(EventType key, std::shared_ptr<Provider> provider, bool overwrite = false);

    std::shared_ptr<Provider> lookup(EventType key);
};
