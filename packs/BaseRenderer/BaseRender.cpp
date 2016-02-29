#include <memory>

#include "render/Renderer.h"
#include "render/opengl/opengl_renderer.h"

#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

class BaseRender {
  public:
    class BaseRenderStaticInit {
        std::shared_ptr<OpenGLRenderer> renderer;

      public:
        BaseRenderStaticInit() {
            std::cout << "BaseRender Constuctor " << std::endl;
            Subscriber* init_subscriber = new Subscriber(this, Function_Cast(&BaseRenderStaticInit::init), false);
            Dispatcher::GetInstance()->AddEventSubscriber(init_subscriber, "EVENT_INITIAL_HOOK");
        }

        void init(std::shared_ptr<void> event_data) {
            int screen_width = 800;
            int screen_height = 600;

            renderer = std::shared_ptr<OpenGLRenderer>(new OpenGLRenderer(screen_width, screen_height));
            std::cout << "BaseRender::Init1 " << renderer << std::endl;
            if (!renderer->initialize()) {
                printf("Graphics Device could not initialize!");
                exit(1);
            }
            std::cout << "BaseRender::Init2 " << renderer << std::endl;
            Dispatcher::GetInstance()->DispatchImmediate("EVENT_RENDER_INIT_SUCCESS", renderer);
        }
    };

    friend class BaseRenderStaticInit;

  public:
    static BaseRender::BaseRenderStaticInit init;
};

BaseRender::BaseRenderStaticInit BaseRender::init;

static BaseRender BASE_RENDERER_NORMAL_NAME;
