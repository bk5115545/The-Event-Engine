#include <memory>

#include "event_system/Provider.h"
#include "event_system/ProviderRegistry.h"
#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

#include <iostream>

class EngineLogging {
  public:
    class EngineLoggingStaticInit : public Provider {

        Subscriber* init_subscriber;

      public:
        EngineLoggingStaticInit() {
            init_subscriber = new Subscriber(this, false);
            init_subscriber->method = std::bind(&EngineLoggingStaticInit::init, this, std::placeholders::_1);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "PROVIDER_INITIAL_HOOK");
        }

        ~EngineLoggingStaticInit() { delete init_subscriber; }

        void init(std::shared_ptr<void> event_data) {
            ProviderRegistry::GetInstance()->provide("Engine Logging", std::shared_ptr<Provider>(this));
        }

        std::shared_ptr<void> provide(std::shared_ptr<void> event_data) {
            try {
                // throw std::exception;
            } catch (std::exception e) {
                // e.printStackTrace();
                std::cout << "ERROR" << std::endl;
            }
        }
    };

    friend class EngineLoggingStaticInit;

  public:
    static EngineLogging::EngineLoggingStaticInit init;
};

EngineLogging::EngineLoggingStaticInit EngineLogging::init;

static EngineLogging ENGINE_LOGGING_NORMAL_NAME;
