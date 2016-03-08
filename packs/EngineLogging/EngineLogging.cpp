#include <memory>

#include "event_system/Provider.h"
#include "event_system/ProviderRegistry.h"
#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class EngineLogging {
  public:
    class EngineLoggingStaticInit : public Provider {

        Subscriber* init_subscriber;

      public:
        EngineLoggingStaticInit() {
            init_subscriber = new Subscriber(this, Function_Cast(&EngineLoggingStaticInit::init), false);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "PROVIDER_INITIAL_HOOK");
        }

        ~EngineLoggingStaticInit() { delete init_subscriber; }

        void init(std::shared_ptr<void> event_data) {
            std::cout << "Logging Provider started." << std::endl;
            ProviderRegistry::GetInstance()->provide("Engine Logging", std::shared_ptr<Provider>(this));
        }

        std::shared_ptr<void> provide(std::shared_ptr<void> event_data) {

            std::shared_ptr<std::vector<std::string>> overrun_city =
                std::static_pointer_cast<std::vector<std::string>>(event_data);

            std::stringstream threadsafe_stream;
            for (std::string str : *overrun_city) {
                threadsafe_stream << str << "\t";
            }
            threadsafe_stream << std::endl;
            std::cout << threadsafe_stream.rdbuf();
        }

        std::shared_ptr<void> get_parameter_data() {
            return std::make_shared<int>(1); // we require 1 argument
        }
    };

    friend class EngineLoggingStaticInit;

  public:
    static EngineLogging::EngineLoggingStaticInit init;
};

__attribute__((used)) EngineLogging::EngineLoggingStaticInit EngineLogging::init;
__attribute__((used)) static EngineLogging ENGINE_LOGGING_NORMAL_NAME;
