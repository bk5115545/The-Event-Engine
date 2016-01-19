#include <memory>

#include "render/renderer.h"
#include "render/opengl/opengl_renderer.h"

#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

class BaseRender {
 public:
    class BaseRenderStaticInit {
        std::shared_ptr<OpenGLRenderer> renderer;
     public:
        BaseRenderStaticInit() {
            Subscriber* init_subscriber = new Subscriber(this, false);
            init_subscriber->method = std::bind(&BaseRenderStaticInit::Init, this, std::placeholders::_1);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "EVENT_INITIAL_HOOK");
        }

        void Init(std::shared_ptr<void> event_data) {
            uint32 screen_width = 800;
            uint32 screen_height = 600;

            renderer = std::shared_ptr<OpenGLRenderer>(new OpenGLRenderer(screen_width, screen_height));

            if (!renderer->Initialize()) {
                printf("Graphics Device could not initialize!");
                exit(1);
            }
            Dispatcher::GetInstance()->DispatchImmediate("EVENT_RENDER_INIT_SUCCESS", renderer);
        }
    };

    friend class BaseRenderStaticInit;
 public:
        static BaseRender::BaseRenderStaticInit init;
};

BaseRender::BaseRenderStaticInit BaseRender::init;

static BaseRender BASE_RENDERER_NORMAL_NAME;
