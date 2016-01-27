#pragma once

#define GLM_FORCE_RADIANS

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "virtual/Component.h"

#include "render/opengl/opengl_renderer.h"
#include "render/opengl/gl_model.h"

#include "core/Actor.h"

#include "event_system/Subscriber.h"
#include "event_system/Dispatcher.h"

class OpenGLRenderer;

class GlDrawable : public Component {
  protected:
    Actor* owner;
    std::vector<Subscriber*> subscribers;

    glm::mat4 model_matrix;

    std::shared_ptr<GLModel> model;

  public:
    GlDrawable(Actor*);
    ~GlDrawable();

    void initialize(std::shared_ptr<Renderer>, std::shared_ptr<GLModel>);
    void process(std::shared_ptr<void> delta_time);
    void draw(glm::mat4);
};
