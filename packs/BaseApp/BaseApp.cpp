#include <memory>

#include "core/app_3d.h"

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

class BaseApp {
 public:
    class BaseAppStaticInit {
        std::shared_ptr<App3D> app;
     public:
        BaseAppStaticInit() {
            Subscriber* init_subscriber = new Subscriber(this, false);
            init_subscriber->method = std::bind(&BaseAppStaticInit::Init, this, std::placeholders::_1);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "EVENT_RENDER_INIT_SUCCESS");
        }

        void Init(std::shared_ptr<void> event_data) {
            app = std::shared_ptr<App3D>(new App3D());
            if (!app->Initialize(std::static_pointer_cast<Renderer>(event_data))) {
                printf("Game could not Initialize!");
                exit(1);
            }

            Dispatcher::GetInstance()->DispatchImmediate("EVENT_APP_INIT_SUCCESS", app);
        }
    };

    friend class BaseAppStaticInit;
 public:
    static BaseApp::BaseAppStaticInit init;
};

BaseApp::BaseAppStaticInit BaseApp::init;

static BaseApp BASE_APP_NORMAL_NAME;
