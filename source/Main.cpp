// Copyright 2015 Casey Megginson and Blaise Koch

#define GLM_FORCE_RADIANS
#undef _GLIBCXX_ATOMIC_BUILTINS

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

// Threaded Events
#include "event_system/ProviderRegistry.h"
#include "event_system/Provider.h"
#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

#include "util/CrossPlatform.h"

#include "virtual/App.h"

class EngineCoreMinimal {
    // TODO(bk5115545) Switch these volatiles to std::atomic<bool>
    volatile bool ready = false;
    volatile bool quit = false;
    std::shared_ptr<App> app = nullptr;

  public:
    EngineCoreMinimal() {
        Subscriber* ready_subscriber = new Subscriber(this, Function_Cast(&EngineCoreMinimal::app_ready), false);
        Dispatcher::GetInstance()->AddEventSubscriber(ready_subscriber, "EVENT_APP_READY");

        Subscriber* shutdown_subscriber = new Subscriber(this, Function_Cast(&EngineCoreMinimal::all_shutdown), false);
        Dispatcher::GetInstance()->AddEventSubscriber(shutdown_subscriber, "EVENT_ALL_SHUTDOWN");
    }

    void app_ready(std::shared_ptr<void> event_data) {
        app = std::static_pointer_cast<App>(event_data);
        Dispatcher::GetInstance()->DispatchImmediate("EVENT_APP_INIT_SUCCESS_LATE", event_data);
        Dispatcher::GetInstance()->Pump();
        Dispatcher::GetInstance()->NonSerialProcess();
        sleep(500);
        ready = true;
    }

    bool is_ready() { return ready && (app != nullptr); }

    void main_loop() {
        //========================================
        // Initialize the random number generator
        //========================================
        srand((unsigned int)time(NULL));

        while (!is_ready()) {
            std::cout << "Waiting on EVENT_APP_READY with std::shared_ptr<App> as argument." << std::endl;
            sleep(500);
        }
        std::cout << "Start main_loop()." << std::endl << std::endl;
        ;
        quit = false;
        while (!quit) {
            // Provide a way for packs to hook into the main loop
            Dispatcher::GetInstance()->DispatchEvent("EVENT_APP_RUN", std::shared_ptr<void>(nullptr));
            Dispatcher::GetInstance()->Pump();
            Dispatcher::GetInstance()->NonSerialProcess();

            while (Dispatcher::GetInstance()->GetApproximateSize() > 2000) {
                Dispatcher::GetInstance()->Pump();
                Dispatcher::GetInstance()->NonSerialProcess();
                sleep(0); // hint to scheduler to yeild processor to other threads
            }

            if (ProviderRegistry::GetInstance()->hasProvider("Engine Logging")) {
                // std::vector<std::string> message;
                // message.push_back(std::string("Main.cpp"));
                // message.push_back(std::string("Test of logging."));

                // ProviderRegistry::GetInstance()
                //    ->lookup("Engine Logging")
                //    ->provide(std::make_shared<std::vector<std::string>>(message));
            }
        }
    }

    void all_shutdown(std::shared_ptr<void> event_data) {
        UNUSED(event_data);
        quit = true;
        std::cout << "\nSHUTDOWN RECIEVED" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    // Kind of the startup sequence.  I need to document this somewhere
    // TODO(bk5115545) Document startup sequence addition of PROVIDER_INITIAL_HOOK

    EngineCoreMinimal engine = EngineCoreMinimal();

    Dispatcher::GetInstance()->DispatchImmediate("PROVIDER_INITIAL_HOOK", std::shared_ptr<void>(nullptr));
    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(1000);

    Dispatcher::GetInstance()->DispatchImmediate("EVENT_INITIAL_HOOK", std::shared_ptr<void>(nullptr));
    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(100);

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(100);

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(100);

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(100);

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(100);

    // Run the engine
    engine.main_loop();

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(600);

    Dispatcher::GetInstance()->Terminate();

    return 0;
}
