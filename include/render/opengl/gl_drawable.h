#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "virtual/component.h"

#include "render/opengl/opengl_renderer.h"
#include "render/opengl/gl_model.h"
#include "render/actor.h"

class OpenGLRenderer;

class GlDrawable : public Component {
protected:
    Actor* owner_;
    std::vector<Subscriber*> subscribers_;

    glm::mat4 model_matrix_;

    GlModel* model_;

public:
    GlDrawable(Actor*);
    ~GlDrawable();

    void Initialize(OpenGLRenderer*, GlModel*);
    void Update(std::shared_ptr<void> delta_time);
    void Draw(glm::mat4);
};
