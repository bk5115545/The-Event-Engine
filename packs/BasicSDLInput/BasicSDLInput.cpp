#include <memory>
#include <map>
#include <utility>
#include <iostream>

#include "SDL.h"

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

#include "util/CrossPlatform.h"

class BasicSDLInput {
  public:
    class BasicSDLInputStaticInit {
        std::map<SDL_Keycode, bool> translations_;
        Subscriber* init_subscriber;

      public:
        BasicSDLInputStaticInit() {
            SDL_Init(0);
            init_subscriber = new Subscriber(this, Function_Cast(&BasicSDLInputStaticInit::init), false);
            // Lets use the late event to see if something else initialized SDL for us
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "EVENT_APP_INIT_SUCCESS_LATE");
        }

        ~BasicSDLInputStaticInit() { delete init_subscriber; }

        void init(std::shared_ptr<void> event_data) {
            // Some kind of SDL_Init bug is happening here that shows that SDL_INIT_EVENTS is already set even tough it
            // isn't

            Uint32 sdl_inited = SDL_WasInit(SDL_INIT_EVERYTHING);
            // if(sdl_inited & SDL_INIT_EVENTS == 0) {
            SDL_InitSubSystem(SDL_INIT_EVENTS);
            //}
            if ((sdl_inited & SDL_INIT_GAMECONTROLLER) == 0) {
                SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
            }
            if ((sdl_inited & SDL_INIT_JOYSTICK) == 0) {
                SDL_InitSubSystem(SDL_INIT_JOYSTICK);
            }

            // If something else is managing events then lets just listen and translate key presses
            if (sdl_inited != 0) {
                std::cout << "BasicSDLInput is controlling SDL Events" << std::endl;

                Subscriber* translating_subscriber =
                    new Subscriber(this, Function_Cast(&BasicSDLInputStaticInit::translate), false);
                Dispatcher::GetInstance()->AddEventSubscriber(translating_subscriber, "EVENT_SDL_EVENT");
            }
            // else {
            std::cout << "BasicSDLInput is translating SDL Events" << std::endl;

            // Otherwise lets control the PollEvent loop
            Subscriber* sdl_event_control =
                new Subscriber(this, Function_Cast(&BasicSDLInputStaticInit::event_loop), false);
            Dispatcher::GetInstance()->AddEventSubscriber(sdl_event_control, "EVENT_APP_RUN");
            //}
        }

        void event_loop(std::shared_ptr<void> event_data) {
            UNUSED(event_data);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    Dispatcher::GetInstance()->DispatchEvent("EVENT_ALL_SHUTDOWN", std::shared_ptr<void>(nullptr));
                }
                // Update the Input Device with the Event
                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    Dispatcher::GetInstance()->DispatchEvent("EVENT_SDL_EVENT", std::make_shared<SDL_Event>(event));
                }
            }
        }

        void translate(std::shared_ptr<void> event_data) {
            SDL_Event* sdl_event = (SDL_Event*)event_data.get();

            if (sdl_event->type == SDL_KEYDOWN) {
                Dispatcher::GetInstance()->DispatchEvent(
                    "EVENT_INPUT",
                    std::make_shared<std::pair<int, bool>>(std::pair<int, bool>(sdl_event->key.keysym.sym, true)));
                // std::cout << "Dispatched EVENT_INPUT " << sdl_event->key.keysym.sym << " true" << std::endl;
            } else if (sdl_event->type == SDL_KEYUP) {
                std::shared_ptr<std::pair<int, bool>> obj =
                    std::make_shared<std::pair<int, bool>>(std::pair<int, bool>(sdl_event->key.keysym.sym, false));
                Dispatcher::GetInstance()->DispatchEvent("EVENT_INPUT", obj);
                // std::cout << "Dispatched EVENT_INPUT " << sdl_event->key.keysym.sym << " false" << std::endl;
            }
        }
    };

    friend class BasicSDLInputStaticInit;
    static BasicSDLInput::BasicSDLInputStaticInit init;
};

BasicSDLInput::BasicSDLInputStaticInit BasicSDLInput::init;
static BasicSDLInput BASIC_SDL_INPUT_NORMAL_NAME;
