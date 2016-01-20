#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"
#include "virtual/Component.h"

class Actor {
  protected:
    glm::vec3 pos;
    // glm::mat4 model_matrix_;

    std::vector<Component*> components;
    std::vector<Subscriber*> subscribers;

  public:
    Actor();
    ~Actor();
    void Initialize(std::string);
    void AddComponent(Component*);
    void Update(std::shared_ptr<void>);

    glm::vec3 get_position();
};
