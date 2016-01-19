#include <iostream>
#include <string>

#include "packs/BaseApp/App3D.h"
#include "render/opengl/gl_drawable.h"

#include "util/CrossPlatform.h"

App3D::App3D() {
    // component_factories_ = nullptr;
    model_store = nullptr;
    timer = nullptr;
    camera = nullptr;
}

App3D::~App3D() {
    // if (component_factories_ != nullptr) {
    //     delete component_factories_;
    //     component_factories_ = nullptr;
    // }
    if (model_store != nullptr) {
        delete model_store;
        model_store = nullptr;
    }
    if (timer != nullptr) {
        delete timer;
        timer = nullptr;
    }
}

bool App3D::initialize(std::shared_ptr<Renderer> rend) {
    renderer = std::dynamic_pointer_cast<OpenGLRenderer>(rend);

    camera = new GlCamera();
    camera->initialize();

    renderer->set_camera(camera);

    model_store = new GlModelStore();
    model_store->loadAssets();

    timer = new Timer();
    timer->Start();

    return true;
}

void App3D::reset() {}

bool App3D::loadLevel(std::string file) {
    Actor *new_actor = new Actor();
    // new_actor->Initialize("cube")
    std::string model = "cube";

    GlDrawable *new_gldrawable = new GlDrawable(new_actor);
    new_gldrawable->initialize(renderer, model_store->search(model));
    Component *new_component = static_cast<Component *>(new_gldrawable);
    new_actor->AddComponent(new_component);

    return true;
}

void App3D::run() {
    update(timer->DeltaTime());
    timer->Update();

    renderer->preDraw();
    renderer->draw();
    renderer->postDraw();
}

void App3D::update(float delta_time) {

    Dispatcher::GetInstance()->DispatchEvent("EVENT_COMPONENT_UPDATE", std::make_shared<float>(delta_time));

    Dispatcher::GetInstance()->Pump();
    Dispatcher::GetInstance()->NonSerialProcess();

    std::cout << "FPS: " << 1 / delta_time << "\r";

    // TODO(bk515545)
    // We have to wait for all threads to finish before terminating this function
    // otherwise updating and rendering might access the same variables
    // simultaniously
    // this is a work around while a better solution is put into Dispatcher
    while (Dispatcher::GetInstance()->QueueSize() > 0) {
        // std::cout << "Thread queue is too full...  waiting a bit" << std::endl;
        sleep(1);
    }
}
