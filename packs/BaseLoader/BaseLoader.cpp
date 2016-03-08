#include <memory>
#include <string>

#include "packs/BaseApp/App3D.h"

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

class BaseLoader {
  public:
    class BaseLoaderStaticInit {
      public:
        BaseLoaderStaticInit() {
            Subscriber* init_subscriber = new Subscriber(this, Function_Cast(&BaseLoaderStaticInit::init), false);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "EVENT_APP_INIT_SUCCESS");
        }

        void init(std::shared_ptr<void> event_data) {
            //========================================
            // Load Level
            //========================================
            App3D* app = (App3D*)event_data.get();
            app->reset();
            std::string level_config_file = "foobar.xml";
            if (!app->loadLevel(level_config_file)) {
                printf("Game could not load level %s: ", level_config_file.c_str());
                exit(1); // this case will leak a lot of memory...
                         // should properly do destructor calls and proper shutdown
            }

            Dispatcher::GetInstance()->DispatchImmediate("EVENT_APP_READY", event_data);
        }
    };

    friend class BaseLoaderStaticInit;

  private:
    static BaseLoader::BaseLoaderStaticInit init;
};

__attribute__((used)) BaseLoader::BaseLoaderStaticInit BaseLoader::init;
__attribute__((used)) static BaseLoader BASE_LOADER_NORMAL_NAME;
