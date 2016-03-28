#include "core/Actor.h"
#include "virtual/Component.h"

#include "event_system/Dispatcher.h"
#include "event_system/Subscriber.h"

Actor::Actor() {
    pos = glm::vec3(rand() % 10, rand() % 10, rand() % 10);

    Subscriber* update_subscriber = new Subscriber(this, Function_Cast(&Actor::Update));
    subscribers.push_back(update_subscriber);
    // simulate load
    Dispatcher::GetInstance()->AddEventSubscriber(update_subscriber, "EVENT_APP_RUN");
}

Actor::~Actor() {
    for (unsigned int i = 0; i < subscribers.size(); i++) {
        delete subscribers.at(i);
    }
}

void Actor::AddComponent(Component* new_component) { components.push_back(new_component); }

void Actor::Update(std::shared_ptr<void> delta_time) {}

glm::vec3 Actor::get_position() { return pos; }
