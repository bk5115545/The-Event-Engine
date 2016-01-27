#include "event_system/ProviderRegistry.h"

#include <map>
#include <memory>

// TODO(bk5115545) convert this to use std::shared_mutex when c++1z is common (don't pull in boost)
#include <mutex>

bool ProviderRegistry::inited = false;
ProviderRegistry* ProviderRegistry::theInstance = nullptr;

ProviderRegistry::ProviderRegistry() { provider_lookup = new std::map<EventType, std::shared_ptr<Provider>>(); }

ProviderRegistry* ProviderRegistry::GetInstance() {
    if (inited) {
        return theInstance;
    } else {
        inited = true;
        theInstance = new ProviderRegistry();
        return theInstance;
    }
}

bool ProviderRegistry::hasProvider(EventType key) {
    std::lock_guard<std::mutex> provider_lock(lookup_mutex);
    return provider_lookup->count(key) > 0;
}

bool ProviderRegistry::provide(EventType key, std::shared_ptr<Provider> provider, bool overwrite) {
    std::lock_guard<std::mutex> provider_lock(lookup_mutex);
    if (!overwrite && provider_lookup->count(key) == 0) {
        provider_lookup->emplace(key, provider);
        return true;
    } else if (overwrite) {
        provider_lookup->emplace(key, provider);
        return true;
    }

    return false;
}

std::shared_ptr<Provider> ProviderRegistry::lookup(EventType key) {
    std::lock_guard<std::mutex> provider_lock(lookup_mutex);
    if (provider_lookup->count(key) > 0) {
        return provider_lookup->at(key);
    } else {
        return std::shared_ptr<Provider>(nullptr);
    }
}
