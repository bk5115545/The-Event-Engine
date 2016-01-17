// Copyright 2015 Casey Megginson and Blaise Koch

#include <ctime>
#include <iostream>
#include <string>

// Threaded Events
#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

// Project Libraries
#include "util/definitions.h"
#include "core/app_3d.h"


class EngineCoreMinimal {
    // TODO(bk5115545) Switch these volatiles to std::atomic<int>
    // LAZY()
    volatile bool ready_ = false;
    volatile bool quit_ =  false;
    std::shared_ptr<App> app_ = nullptr;

 public:
     EngineCoreMinimal() {
         Subscriber* ready_subscriber = new Subscriber(this,false);
         ready_subscriber->method = std::bind(&EngineCoreMinimal::app_ready, this, std::placeholders::_1);
         Dispatcher::GetInstance()->AddEventSubscriber(ready_subscriber, "EVENT_APP_READY");

         Subscriber* shutdown_subscriber = new Subscriber(this,false);
         shutdown_subscriber->method = std::bind(&EngineCoreMinimal::all_shutdown, this, std::placeholders::_1);
         Dispatcher::GetInstance()->AddEventSubscriber(shutdown_subscriber, "EVENT_ALL_SHUTDOWN");
     }

    void app_ready(std::shared_ptr<void> event_data) {
        app_ = std::static_pointer_cast<App>(event_data);
        Dispatcher::GetInstance()->DispatchImmediate("EVENT_APP_INIT_SUCCESS_LATE", event_data);
        Dispatcher::GetInstance()->Pump();
        Dispatcher::GetInstance()->NonSerialProcess();
        ready_ = true;
    }

    bool ready() {
        return ready_ || app_ != nullptr;
    }

    void main_loop() {
        //========================================
        // Initialize the random number generator
        //========================================
        srand((unsigned int)time(NULL));

        quit_ = false;
        while (!quit_) {
            // Provide a way for packs to hook into the main loop
            Dispatcher::GetInstance()->DispatchEvent("EVENT_APP_RUN", std::shared_ptr<void>(nullptr));
            Dispatcher::GetInstance()->Pump();
            Dispatcher::GetInstance()->NonSerialProcess();
            app_->Run();
        }
    }

    void all_shutdown(std::shared_ptr<void> event_data) {
        UNUSED(event_data);
        quit_ = true;
        std::cout << "SHUTDOWN RECIEVED" << std::endl;
        //========================================
        // Clean-up
        //========================================

    }
};

int main(int argc, char* argv[]) {
    UNUSED(argc); UNUSED(argv);

    EngineCoreMinimal engine = EngineCoreMinimal();
    Dispatcher::GetInstance()->DispatchImmediate("EVENT_RENDERER_INIT", std::shared_ptr<void>(nullptr));
    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();
    sleep(1000);

    Dispatcher::GetInstance()->NonSerialProcess();

    // Run the engine
    engine.main_loop();

    while(Dispatcher::GetInstance()->QueueSize() > 0) {
        Dispatcher::GetInstance()->Pump();
        Dispatcher::GetInstance()->NonSerialProcess();
        sleep(600);
    }

    Dispatcher::GetInstance()->Terminate();

    return 0;
}
