#include <iostream>
#include <string>

#include "packs/BaseApp/App3D.h"
#include "render/opengl/gl_drawable.h"

#include "util/CrossPlatform.h"

App3D::App3D() {
    model_store = nullptr;
    timer = nullptr;
    camera = nullptr;
}

App3D::~App3D() {
    for (int i = 0; i < actors.size(); i++) {
        if (actors[i] != nullptr)
            delete actors.at(i);
        actors[i] = nullptr;
    }

    if (camera != nullptr) {
        delete camera;
        camera = nullptr;
    }

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
    std::cout << "App3D::initialize1 " << renderer << std::endl;

    renderer = std::dynamic_pointer_cast<OpenGLRenderer>(rend);

    camera = new GLCamera();
    camera->initialize();

    std::cout << "App3D::initialize2 " << renderer << std::endl;

    renderer->set_camera(camera);

    model_store = new GLModelStore();
    model_store->loadAssets();

    timer = new Timer();
    timer->start();

    Subscriber* run_subscriber = new Subscriber(this, Function_Cast(&App3D::run), false);
    Dispatcher::GetInstance()->AddEventSubscriber(run_subscriber, "EVENT_APP_RUN");

    return true;
}

void App3D::reset() {}

bool App3D::loadLevel(std::string file) {
    UNUSED(file);

    for (int i = 0; i < 1000; i++) {
        Actor* new_actor = new Actor();
        actors.push_back(new_actor);

        GlDrawable* new_gldrawable = new GlDrawable(new_actor);
        new_gldrawable->initialize(renderer, model_store->search("cube"));
        Component* new_component = static_cast<Component*>(new_gldrawable);
        new_actor->AddComponent(new_component);
    }

    return true;
}

void App3D::run(std::shared_ptr<void> event_data) {
    UNUSED(event_data);

    update(std::make_shared<float>(timer->get_deltaTime()));
    timer->update();

    renderer->preDraw();
    renderer->draw();
    renderer->postDraw();
}

void App3D::update(std::shared_ptr<void> event_data) {
    if (event_data.get() == nullptr) {
        std::cout << "Null time delta caught at App3D::update before it was repropogated." << std::endl;
        return;
    }
    Dispatcher::GetInstance()->DispatchEvent("EVENT_COMPONENT_UPDATE", event_data);

    std::cout << "Frame Time: " << *(float*)(event_data.get()) * 1000 << " FPS: " << 1 / *(float*)(event_data.get())
              << "\r";
}
