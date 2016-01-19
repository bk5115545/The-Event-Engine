#include "render/opengl/gl_drawable.h"

GlDrawable::GlDrawable(Actor *owner) {
    this->owner = owner;

    Subscriber *s = new Subscriber(this);
    s->method = std::bind(&GlDrawable::process, this, std::placeholders::_1);
    Dispatcher::GetInstance()->AddEventSubscriber(s, "EVENT_COMPONENT_UPDATE");
    subscribers.push_back(s);
}

GlDrawable::~GlDrawable() { owner = nullptr; }

void GlDrawable::initialize(std::shared_ptr<Renderer> renderer, std::shared_ptr<GlModel> model) {
    ((OpenGLRenderer *)renderer.get())->addModel(this);
    this->model = model;
}

void GlDrawable::process(std::shared_ptr<void> delta_time) {
    UNUSED(delta_time);
    model_matrix = glm::translate(glm::mat4(1.0f), owner->get_position());
}

void GlDrawable::draw(glm::mat4 view_projection) {
    glm::mat4 model_view_projection = view_projection * model_matrix;
    model->draw(model_view_projection);
}
