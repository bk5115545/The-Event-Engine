#include "core/Actor.h"
#include "virtual/Component.h"

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

Actor::Actor() {
    pos = glm::vec3(0.0, 0.0, 0.0);

    Subscriber* update_subscriber = new Subscriber(this);
    update_subscriber->method = std::bind(&Actor::Update, this, std::placeholders::_1);
    Dispatcher::GetInstance()->AddEventSubscriber(update_subscriber, "EVENT_ACTOR_UPDATE");
}

Actor::~Actor() {}

// void Actor::Initialize(std::string name) {
//
// }

void Actor::AddComponent(Component* new_component) { components.push_back(new_component); }

void Actor::Update(std::shared_ptr<void> delta_time) {}

glm::vec3 Actor::get_position() { return pos; }
