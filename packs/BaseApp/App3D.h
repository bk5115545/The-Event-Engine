#pragma once

#include <list>
#include <memory>
#include <string>

#include "core/Actor.h"

#include "render/opengl/gl_camera.h"
#include "render/opengl/gl_model_store.h"
#include "render/opengl/opengl_renderer.h"

#include "util/Timer.h"

#include "virtual/App.h"

class App3D : public App {
  protected:
    Timer* timer;

    std::shared_ptr<OpenGLRenderer> renderer;
    GlCamera* camera;
    GlModelStore* model_store;

    std::list<Actor*> actors;

  public:
    App3D();
    ~App3D();
    bool initialize(std::shared_ptr<Renderer> renderer);
    void reset();
    bool loadLevel(std::string config_file_name);
    void run(std::shared_ptr<void> event_data);
    void update(std::shared_ptr<void> event_data);
};
