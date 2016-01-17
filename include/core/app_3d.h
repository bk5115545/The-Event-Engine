#pragma once

#include <list>
#include <string>

// #include "art_library/ComponentLibrary.h"
#include "render/opengl/gl_model_store.h"

#include "util/timer.h"

#include "render/opengl/gl_camera.h"
#include "render/opengl/opengl_renderer.h"
#include "render/actor.h"

#include "virtual/component.h"
#include "virtual/app.h"

class App3D : public App {
protected:
    // ComponentLibrary* component_factories_;
    Timer* timer_;

    std::shared_ptr<OpenGLRenderer> renderer_;
    GlCamera* camera_;
    GlModelStore* model_store_;

    std::list<Actor*> actors_;

public:
    App3D();
    ~App3D();
    bool Initialize(std::shared_ptr<Renderer>);
    void Reset();
    bool LoadLevel(std::string);
    void Run();
    void Update(float32);
};
